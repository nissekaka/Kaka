#include "stdafx.h"
#include "Terrain.h"
#include "Core/Graphics/Drawable/Vertex.h"
#include "External/include/imgui/imgui.h"

#include "TGP/uppgift05_helper.h"
#include "TGP/FastNoiseLite.h"

#include <cmath>
#include <random>

#include "Graphics/ShaderFactory.h"

namespace Kaka
{
	float GetHeight(const float aX, const float aZ)
	{
		return 0.1f * (aZ * sinf(0.4f * aX) + aX * cosf(0.4f * aZ));
	}

	void Terrain::Init(const Graphics& aGfx, const int aSize)
	{
		FastNoiseLite noiseGenerator;
		noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
		noiseGenerator.SetFractalType(FastNoiseLite::FractalType_Ridged);
		noiseGenerator.SetFrequency(0.0012f);
		noiseGenerator.SetFractalOctaves(8);
		noiseGenerator.SetFractalLacunarity(1.8f);
		noiseGenerator.SetFractalGain(0.5f);

		constexpr float heightMul = 100.0f;
		constexpr float uvFactor = 80.0f;

		std::vector<Vertex> terrainVertices;
		std::vector<unsigned short> terrainIndices;

		// Setup subsets
		constexpr int subsetSize = 8192; // Number of indices in each subset
		const int verticesPerSubsetRow = static_cast<int>(std::sqrt(subsetSize));
		constexpr bool shouldFillGaps = true;

		terrainSize = aSize;

		// Make terrain size smaller to create perfect squares of subsets
		while (true)
		{
			if ((terrainSize + verticesPerSubsetRow - 1) % verticesPerSubsetRow == 0)
			{
				break;
			}
			--terrainSize;
		}

		for (int z = 0; z < terrainSize; ++z)
		{
			for (int x = 0; x < terrainSize; ++x)
			{
				Vertex vertex = {};
				float noiseValue = noiseGenerator.GetNoise(static_cast<float>(x), static_cast<float>(z));
				vertex.position = DirectX::XMFLOAT3(static_cast<float>(x), noiseValue * heightMul,
				                                    static_cast<float>(z));
				vertex.normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
				vertex.texCoord = DirectX::XMFLOAT2(static_cast<float>(x) / uvFactor,
				                                    static_cast<float>(z) / uvFactor);
				vertex.tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
				vertex.bitangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
				terrainVertices.push_back(vertex);
			}
		}

		OutputDebugStringA("\nDone generating...");

		constexpr float smoothingFactor = 1.0f;
		float heightThreshold = 0.0f;
		int smoothingIterations = 15;

		for (int i = 0; i < smoothingIterations; ++i)
		{
			for (int z = 0; z < terrainSize; ++z)
			{
				for (int x = 0; x < terrainSize; ++x)
				{
					const int currentIndex = z * terrainSize + x;
					const int indexTop = (z - 1) * terrainSize + x;
					const int indexBottom = (z + 1) * terrainSize + x;
					const int indexLeft = z * terrainSize + (x - 1);
					const int indexRight = z * terrainSize + (x + 1);

					if (terrainVertices[currentIndex].position.y > heightThreshold)
					{
						continue;
					}

					// Calculate average position of neighbouring vertices
					DirectX::XMFLOAT3 averagePosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
					int numNeighbours = 0;

					if (z > 0)
					{
						averagePosition.y += terrainVertices[indexTop].position.y;
						numNeighbours++;
					}
					if (z < terrainSize - 1)
					{
						averagePosition.y += terrainVertices[indexBottom].position.y;
						numNeighbours++;
					}
					if (x > 0)
					{
						averagePosition.y += terrainVertices[indexLeft].position.y;
						numNeighbours++;
					}
					if (x < terrainSize - 1)
					{
						averagePosition.y += terrainVertices[indexRight].position.y;
						numNeighbours++;
					}
					averagePosition.y /= numNeighbours;

					DirectX::XMFLOAT3& currentPosition = terrainVertices[currentIndex].position;
					DirectX::XMFLOAT3 smoothedPosition = DirectX::XMFLOAT3(
						currentPosition.x,
						currentPosition.y + (averagePosition.y - currentPosition.y) * smoothingFactor,
						currentPosition.z
					);

					// Update the vertex position
					terrainVertices[currentIndex].position = smoothedPosition;
				}
			}
		}

		OutputDebugStringA("\nDone smoothing...");

		// Calculate normals
		for (int z = 0; z < terrainSize; ++z)
		{
			for (int x = 0; x < terrainSize; ++x)
			{
				const int currentIndex = z * terrainSize + x;
				const int indexTop = (z - 1) * terrainSize + x;
				const int indexBottom = (z + 1) * terrainSize + x;
				const int indexLeft = z * terrainSize + (x - 1);
				const int indexRight = z * terrainSize + (x + 1);

				const bool isBorder = (x == 0 || x == terrainSize - 1 || z == 0 || z == terrainSize - 1);

				DirectX::XMFLOAT3& currentPosition = terrainVertices[currentIndex].position;
				DirectX::XMFLOAT3 topPosition = (z > 0) ? terrainVertices[indexTop].position : currentPosition;
				DirectX::XMFLOAT3 bottomPosition = (z < terrainSize - 1)
					                                   ? terrainVertices[indexBottom].position
					                                   : currentPosition;
				DirectX::XMFLOAT3 leftPosition = (x > 0) ? terrainVertices[indexLeft].position : currentPosition;
				DirectX::XMFLOAT3 rightPosition = (x < terrainSize - 1)
					                                  ? terrainVertices[indexRight].position
					                                  : currentPosition;

				DirectX::XMVECTOR edge1 = DirectX::XMVectorSubtract(XMLoadFloat3(&topPosition),
				                                                    XMLoadFloat3(&bottomPosition));
				DirectX::XMVECTOR edge2 = DirectX::XMVectorSubtract(XMLoadFloat3(&leftPosition),
				                                                    XMLoadFloat3(&rightPosition));
				DirectX::XMVECTOR normal = DirectX::XMVector3Cross(edge1, edge2);

				normal = DirectX::XMVector3Normalize(normal);
				DirectX::XMStoreFloat3(&terrainVertices[currentIndex].normal, normal);

				if (isBorder)
				{
					DirectX::XMStoreFloat3(&terrainVertices[currentIndex].normal, {0, 1, 0});
				}
				else
				{
					edge1 = DirectX::XMVectorSubtract(XMLoadFloat3(&leftPosition), XMLoadFloat3(&currentPosition));
					edge2 = DirectX::XMVectorSubtract(XMLoadFloat3(&bottomPosition), XMLoadFloat3(&currentPosition));

					DirectX::XMFLOAT2 deltaUV1 = DirectX::XMFLOAT2(1.0f, 0.0f);
					DirectX::XMFLOAT2 deltaUV2 = DirectX::XMFLOAT2(0.0f, 1.0f);

					// Calculate tangent
					DirectX::XMVECTOR tangent;
					tangent = DirectX::XMVectorScale(
						DirectX::XMVectorSubtract(DirectX::XMVectorScale(edge1, deltaUV2.y),
						                          DirectX::XMVectorScale(edge2, deltaUV1.y)),
						1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y));
					tangent = DirectX::XMVector3Normalize(tangent);

					// Calculate bitangent
					DirectX::XMVECTOR bitangent;
					bitangent = DirectX::XMVectorScale(
						DirectX::XMVectorSubtract(DirectX::XMVectorScale(edge2, deltaUV1.x),
						                          DirectX::XMVectorScale(edge1, deltaUV2.x)),
						1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y));
					bitangent = DirectX::XMVector3Normalize(bitangent);

					DirectX::XMStoreFloat3(&terrainVertices[currentIndex].tangent, tangent);
					DirectX::XMStoreFloat3(&terrainVertices[currentIndex].bitangent, bitangent);
				}
			}
		}

		OutputDebugStringA("\nDone with normals...");

		for (int z = 0; z < terrainSize - 1; ++z)
		{
			for (int x = 0; x < terrainSize - 1; ++x)
			{
				const int index = z * terrainSize + x;

				terrainIndices.push_back(static_cast<unsigned short>(index));
				terrainIndices.push_back(static_cast<unsigned short>(index + terrainSize));
				terrainIndices.push_back(static_cast<unsigned short>(index + 1));

				terrainIndices.push_back(static_cast<unsigned short>(index + terrainSize));
				terrainIndices.push_back(static_cast<unsigned short>(index + terrainSize + 1));
				terrainIndices.push_back(static_cast<unsigned short>(index + 1));
			}
		}

		OutputDebugStringA("\nPushed indices...");

		const int numSubsets = (terrainSize + verticesPerSubsetRow - 1) / verticesPerSubsetRow;
		const int numVertices = static_cast<int>(terrainVertices.size());

		terrainSubsets.resize(numSubsets * numSubsets);

		for (int i = 0; i < numSubsets; ++i)
		{
			for (int j = 0; j < numSubsets; ++j)
			{
				int currentIndex = i * numSubsets + j;
				const bool isLastZ = i == numSubsets - 1;
				const bool isLastX = j == numSubsets - 1;

				const int startZ = i * verticesPerSubsetRow;
				const int startX = j * verticesPerSubsetRow;
				int endZ = 0;
				int endX = 0;

				int verticesPerZ = verticesPerSubsetRow;
				int verticesPerX = verticesPerSubsetRow;

				if (shouldFillGaps)
				{
					if (isLastZ && !isLastX)
					{
						verticesPerX++;
					}

					if (!isLastZ && isLastX)
					{
						verticesPerZ++;
					}

					if (!isLastZ && !isLastX)
					{
						verticesPerZ++;
						verticesPerX++;
					}
				}

				endZ = (std::min)(startZ + verticesPerZ, terrainSize);
				endX = (std::min)(startX + verticesPerX, terrainSize);

				for (int z = startZ; z < endZ; ++z)
				{
					for (int x = startX; x < endX; ++x)
					{
						const int index = z * terrainSize + x;

						// Check if the index is within the terrain range
						if (index < numVertices)
						{
							terrainSubsets[currentIndex].vertices.push_back(terrainVertices[index]);

							if (terrainSubsets[currentIndex].vertices.back().position.x < terrainSubsets[currentIndex].
							                                                              bounds.min.x)
							{
								terrainSubsets[currentIndex].bounds.min.x = terrainSubsets[currentIndex].vertices.back()
								                                                                        .position.x;
							}
							if (terrainSubsets[currentIndex].vertices.back().position.x > terrainSubsets[currentIndex].
							                                                              bounds.max.x)
							{
								terrainSubsets[currentIndex].bounds.max.x = terrainSubsets[currentIndex].vertices.back()
								                                                                        .position.x;
							}

							if (terrainSubsets[currentIndex].vertices.back().position.y < terrainSubsets[currentIndex].
							                                                              bounds.min.y)
							{
								terrainSubsets[currentIndex].bounds.min.y = terrainSubsets[currentIndex].vertices.back()
								                                                                        .position.y;
							}
							if (terrainSubsets[currentIndex].vertices.back().position.y > terrainSubsets[currentIndex].
							                                                              bounds.max.y)
							{
								terrainSubsets[currentIndex].bounds.max.y = terrainSubsets[currentIndex].vertices.back()
								                                                                        .position.y;
							}

							if (terrainSubsets[currentIndex].vertices.back().position.z < terrainSubsets[currentIndex].
							                                                              bounds.min.z)
							{
								terrainSubsets[currentIndex].bounds.min.z = terrainSubsets[currentIndex].vertices.back()
								                                                                        .position.z;
							}
							if (terrainSubsets[currentIndex].vertices.back().position.z > terrainSubsets[currentIndex].
							                                                              bounds.max.z)
							{
								terrainSubsets[currentIndex].bounds.max.z = terrainSubsets[currentIndex].vertices.back()
								                                                                        .position.z;
							}
						}
					}
				}

				int centerIndex = static_cast<int>(terrainSubsets[currentIndex].vertices.size()) / 2;
				terrainSubsets[currentIndex].center = terrainSubsets[currentIndex].vertices[centerIndex].
					position;

				int vertZ = verticesPerSubsetRow;
				int vertX = verticesPerSubsetRow;

				if (shouldFillGaps)
				{
					if (!isLastZ)
					{
						vertZ++;
					}

					if (!isLastX)
					{
						vertX++;
					}
				}

				for (int z = 0; z < vertZ - 1; ++z)
				{
					for (int x = 0; x < vertX - 1; ++x)
					{
						const int topLeftIndex = z * vertX + x;
						const int topRightIndex = topLeftIndex + 1;
						const int bottomLeftIndex = (z + 1) * vertX + x;
						const int bottomRightIndex = bottomLeftIndex + 1;

						// Check if all indices are within the subset range
						if (topLeftIndex < terrainSubsets[currentIndex].vertices.size() &&
							topRightIndex < terrainSubsets[currentIndex].vertices.size() &&
							bottomLeftIndex < terrainSubsets[currentIndex].vertices.size() &&
							bottomRightIndex < terrainSubsets[currentIndex].vertices.size())
						{
							terrainSubsets[currentIndex].indices.push_back(
								static_cast<unsigned short>(topLeftIndex));
							terrainSubsets[currentIndex].indices.push_back(
								static_cast<unsigned short>(bottomLeftIndex));
							terrainSubsets[currentIndex].indices.push_back(
								static_cast<unsigned short>(topRightIndex));

							terrainSubsets[currentIndex].indices.push_back(
								static_cast<unsigned short>(topRightIndex));
							terrainSubsets[currentIndex].indices.push_back(
								static_cast<unsigned short>(bottomLeftIndex));
							terrainSubsets[currentIndex].indices.push_back(
								static_cast<unsigned short>(bottomRightIndex));
						}
					}
				}
			}
		}

		OutputDebugStringA("\nMoved to subsets...");

		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Grass\\Grass_c.png");
		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Grass\\Grass_n.png");
		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Grass\\Grass_m.png");
		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Rock\\Rock_c.png");
		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Rock\\Rock_n.png");
		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Rock\\Rock_m.png");
		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Snow\\Snow_c.png");
		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Snow\\Snow_n.png");
		texture.LoadTextureFromPath(aGfx, "Assets\\Textures\\Snow\\Snow_m.png");

		OutputDebugStringA("\nLoaded textures...");

		sampler.Init(aGfx, 0u);

		for (auto& subset : terrainSubsets)
		{
			if (subset.indices.empty())
			{
				continue;
			}

			subset.vertexBuffer.Init(aGfx, subset.vertices);
			subset.indexBuffer.Init(aGfx, subset.indices);
		}

		pixelShader = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\TerrainPBR_PS.cso");
		vertexShader = ShaderFactory::GetVertexShader(aGfx, L"Shaders\\TerrainReflect_VS.cso");

		inputLayout.Init(aGfx, ied, vertexShader->GetBytecode());
		topology.Init(aGfx, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		rasterizer.Init(aGfx);
		depthStencil.Init(aGfx, DepthStencil::Mode::Write);

		OutputDebugStringA("\nDone!");
	}

	void Terrain::Draw(Graphics& aGfx)
	{
		sampler.Bind(aGfx);
		texture.Bind(aGfx);

		if (aGfx.HasPixelShaderOverride())
		{
			aGfx.GetPixelShaderOverride()->Bind(aGfx);
		}
		else
		{
			pixelShader->Bind(aGfx);

			PixelConstantBuffer<PSMaterialConstant> psConstantBuffer(aGfx, pmc, 0u);
			psConstantBuffer.Bind(aGfx);
		}

		if (aGfx.HasVertexShaderOverride())
		{
			aGfx.GetVertexShaderOverride()->Bind(aGfx);
		}
		else
		{
			vertexShader->Bind(aGfx);
		}

		inputLayout.Bind(aGfx);
		topology.Bind(aGfx);
		rasterizer.Bind(aGfx);
		depthStencil.Bind(aGfx);

		for (int i = 0; i < terrainSubsets.size(); ++i)
		{
			if (!aGfx.IsBoundingBoxInFrustum(terrainSubsets[i].bounds.min, terrainSubsets[i].bounds.max))
			{
				continue;
			}
			if (terrainSubsets[i].indices.empty())
			{
				continue;
			}
			PixelConstantBuffer<PackedLightData> packedLightBuffer(aGfx, terrainSubsets[i].packedLightData, 9u);
			packedLightBuffer.Bind(aGfx);

			terrainSubsets[i].vertexBuffer.Bind(aGfx);
			terrainSubsets[i].indexBuffer.Bind(aGfx);

			TransformConstantBuffer transformConstantBuffer(aGfx, *this, 0u);
			transformConstantBuffer.Bind(aGfx);

			aGfx.DrawIndexed(static_cast<UINT>(std::size(terrainSubsets[i].indices)));
		}

		// Unbind shader resources
		ID3D11ShaderResourceView* nullSRVs[9] = {nullptr};
		aGfx.pContext->PSSetShaderResources(2u, 9u, nullSRVs);
	}

	void Terrain::SetPosition(const DirectX::XMFLOAT3 aPosition)
	{
		transform.x = aPosition.x;
		transform.y = aPosition.y;
		transform.z = aPosition.z;
	}

	void Terrain::Move(const DirectX::XMFLOAT3 aDistance)
	{
		transform.x += aDistance.x;
		transform.y += aDistance.y;
		transform.z += aDistance.z;
	}

	void Terrain::FlipScale(const float aHeight, const bool aReset)
	{
		transform.scale.y *= -1.0f;
		if (aReset)
		{
			transform.y -= aHeight * 2.0f;
		}
		else
		{
			transform.y += aHeight * 2.0f;
		}
	}

	void Terrain::SetReflectShader(const Graphics& aGfx, const bool aValue)
	{
		if (aValue)
		{
			vertexShader = ShaderFactory::GetVertexShader(aGfx, L"Shaders\\TerrainReflect_VS.cso");
		}
		else
		{
			vertexShader = ShaderFactory::GetVertexShader(aGfx, L"Shaders\\TerrainPBR_VS.cso");
		}
	}

	void Terrain::SetCullingMode(const eCullingMode aMode)
	{
		rasterizer.SetCullingMode(aMode);
	}

	DirectX::XMFLOAT3 Terrain::GetRandomVertexPosition() const
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<> sDist(0, (int)terrainSubsets.size() - 1);
		const int subsetIndex = sDist(mt);
		std::uniform_int_distribution<> vDist(0, (int)terrainSubsets[subsetIndex].vertices.size() - 1);
		const int vertIndex = vDist(mt);

		float x = terrainSubsets[subsetIndex].vertices[vertIndex].position.x + transform.x;
		float y = terrainSubsets[subsetIndex].vertices[vertIndex].position.y + transform.y;
		float z = terrainSubsets[subsetIndex].vertices[vertIndex].position.z + transform.z;
		return {x, y, z};
	}

	DirectX::XMMATRIX Terrain::GetTransform() const
	{
		return DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
			DirectX::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
			DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);
	}

	std::vector<TerrainSubset>& Terrain::GetTerrainSubsets()
	{
		return terrainSubsets;
	}

	int Terrain::GetSize() const
	{
		return terrainSize;
	}

	void Terrain::ShowControlWindow(const char* aWindowName)
	{
		aWindowName = aWindowName ? aWindowName : "Terrain";

		if (ImGui::Begin(aWindowName))
		{
			ImGui::Text("Orientation");
			ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
			ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
			ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);
			ImGui::Text("Position");
			ImGui::DragFloat3("XYZ", &transform.x);
			ImGui::Text("Scale");
			ImGui::DragFloat("XYZ", &transform.scale.x, 0.1f, 0.0f, 10.0f, "%.1f");
		}
		ImGui::End();
	}
}
