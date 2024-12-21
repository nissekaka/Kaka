#include "stdafx.h"
#include "Sprite.h"

#include <random>
#include <thread>

#include "ModelLoader.h"
#include "Core/Graphics/Drawable/Vertex.h"
#include "Core/Utility/Camera.h"
#include "Core/Utility/KakaMath.h"
#include <External/include/imgui/imgui.h>

namespace Kaka
{
	static constexpr float MAX_RANGE_TO_CAMERA = 15.0f;

	void Sprite::Init(const Graphics& aGfx, const float aSize, const unsigned int aNumberOfSprites, bool aIsVfx, const std::string& aFile, const DirectX::XMFLOAT3 aCameraPosition)
	{
		constexpr float uvFactor = 1.0f;

		updateIncrease = aNumberOfSprites / 1;

		SpriteVertex v0 = {};
		v0.position = DirectX::XMFLOAT4(-aSize, 0.0f, aSize, 1.0f);
		v0.texCoord = DirectX::XMFLOAT2(0.0f / uvFactor, 0.0f / uvFactor);
		v0.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		v0.tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		v0.bitangent = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		vertices.push_back(v0);

		SpriteVertex v1 = {};
		v1.position = DirectX::XMFLOAT4(aSize, 0.0f, aSize, 1.0f);
		v1.texCoord = DirectX::XMFLOAT2(0.0f / uvFactor, 1.0f / uvFactor);
		v1.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		v1.tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		v1.bitangent = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		vertices.push_back(v1);

		SpriteVertex v2 = {};
		v2.position = DirectX::XMFLOAT4(aSize, 0.0f, -aSize, 1.0f);
		v2.texCoord = DirectX::XMFLOAT2(1.0f / uvFactor, 1.0f / uvFactor);
		v2.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		v2.tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		v2.bitangent = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		vertices.push_back(v2);

		SpriteVertex v3 = {};
		v3.position = DirectX::XMFLOAT4(-aSize, 0.0f, -aSize, 1.0f);
		v3.texCoord = DirectX::XMFLOAT2(1.0f / uvFactor, 0.0f / uvFactor);
		v3.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		v3.tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		v3.bitangent = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		vertices.push_back(v3);

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(0);
		indices.push_back(2);
		indices.push_back(3);

		texture = ModelLoader::LoadTexture(aGfx, aFile, 1u);

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(SpriteVertex) * 4;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = vertices.data();

		ID3D11Device* device = aGfx.pDevice.Get();
		HRESULT result = device->CreateBuffer(&bufferDesc, &subresourceData, &vertexBuffer);
		assert(SUCCEEDED(result));

		indexBuffer.Init(aGfx, indices);

		vertexShader = ShaderFactory::GetVertexShader(aGfx, L"Shaders\\Sprite_VS.cso");

		vfxPixelShader = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Sprite_PS.cso");
		deferredPixelShader = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Sprite_Deferred_PS.cso");

		computeShader = ShaderFactory::GetComputeShader(aGfx, L"Shaders\\DustParticles_CS.cso");

		if (aIsVfx)
		{
			pixelShader = vfxPixelShader;
		}
		else
		{
			pixelShader = deferredPixelShader;
		}

		inputLayout.Init(aGfx, ied, vertexShader->GetBytecode());

		instanceData.resize(aNumberOfSprites);
		std::vector<ParticleData> particleData = {};
		particleData.resize(aNumberOfSprites);
		instanceIds.resize(aNumberOfSprites);

		std::random_device rd;
		std::mt19937 gen(rd());

		for (unsigned int i = 0; i < instanceData.size(); ++i)
		{
			instanceIds[i] = i;
			// Set random position between -20, 0, -20 and 20, 0, 20
			std::uniform_real_distribution<float> xDist(-MAX_RANGE_TO_CAMERA, MAX_RANGE_TO_CAMERA);
			std::uniform_real_distribution<float> yDist(-MAX_RANGE_TO_CAMERA, MAX_RANGE_TO_CAMERA);
			std::uniform_real_distribution<float> zDist(-MAX_RANGE_TO_CAMERA, MAX_RANGE_TO_CAMERA);
			std::uniform_real_distribution<float> sizeDist(0.5f, 1.25f);
			std::uniform_real_distribution<float> alphaDist(0.05f, 0.4f);
			std::uniform_real_distribution<float> fadeDist(5.0f, 10.0f);
			float scale = sizeDist(gen);
			instanceData[i].instanceTransform = DirectX::XMMatrixScaling(scale, scale, scale);
			particleData[i].colour = {1.0f, 1.0f, 1.0f, alphaDist(gen)};
			instanceData[i].colour = particleData[i].colour;

			DirectX::XMFLOAT3 position = {aCameraPosition.x + xDist(gen), aCameraPosition.y + yDist(gen), aCameraPosition.z + zDist(gen)};

			SetPosition(position, i);
			particleData[i].startPosition = GetPosition(i);

			std::uniform_real_distribution<float> radiusDist(0.5f, 10.0f);
			std::uniform_real_distribution<float> speedDist(0.01f, 0.1f);
			particleData[i].travelRadius = radiusDist(gen);
			particleData[i].travelSpeed.x = speedDist(gen);
			particleData[i].travelSpeed.y = speedDist(gen);
			particleData[i].travelSpeed.z = speedDist(gen);
			particleData[i].travelAngle.x = 0;
			particleData[i].travelAngle.y = PI / 2.0f;
			particleData[i].travelAngle.z = PI;
			particleData[i].fadeSpeed = fadeDist(gen);
		}

		updateCounter += updateIncrease;

		const unsigned int instanceCount = instanceData.size();
		{
			// Create instance buffer
			D3D11_BUFFER_DESC instanceBufferDesc = {};


			instanceBufferDesc.ByteWidth = sizeof(unsigned int) * instanceCount;
			instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = instanceIds.data();
			result = aGfx.pDevice->CreateBuffer(&instanceBufferDesc, nullptr, &instanceBuffer);
			assert(SUCCEEDED(result));
		}

		/// ---------- [ Compute stuff ] ----------

		/// Transform buffer

		{
			// Buffer in GPU memory

			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS |
				D3D11_BIND_SHADER_RESOURCE;
			descGPUBuffer.ByteWidth = sizeof(InstanceData) * instanceCount;
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(InstanceData);

			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = instanceData.data();
			result = aGfx.pDevice->CreateBuffer(&descGPUBuffer, &initData, &instanceCompBuffer);
			assert(SUCCEEDED(result));

			// Shader view

			D3D11_BUFFER_DESC descBuf;
			ZeroMemory(&descBuf, sizeof(descBuf));
			instanceCompBuffer->GetDesc(&descBuf);

			D3D11_SHADER_RESOURCE_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			descView.BufferEx.FirstElement = 0;

			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
			result = aGfx.pDevice->CreateShaderResourceView(instanceCompBuffer, &descView, &srcTransformSRV);
			assert(SUCCEEDED(result));

			// Unordered access view

			D3D11_BUFFER_DESC descUAVBuf;
			ZeroMemory(&descUAVBuf, sizeof(descUAVBuf));
			instanceCompBuffer->GetDesc(&descUAVBuf);

			D3D11_UNORDERED_ACCESS_VIEW_DESC descUAVView;
			ZeroMemory(&descUAVView, sizeof(descUAVView));
			descUAVView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			descUAVView.Buffer.FirstElement = 0;

			// Format must be must be DXGI_FORMAT_UNKNOWN, when creating 
			// a View of a Structured Buffer
			descUAVView.Format = DXGI_FORMAT_UNKNOWN;
			descUAVView.Buffer.NumElements = descUAVBuf.ByteWidth / descUAVBuf.StructureByteStride;
			result = aGfx.pDevice->CreateUnorderedAccessView(instanceCompBuffer, &descUAVView, &instanceUAV);
			assert(SUCCEEDED(result));
		}

		/// Particle buffer

		{
			// Buffer in GPU memory

			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS |
				D3D11_BIND_SHADER_RESOURCE;
			descGPUBuffer.ByteWidth = sizeof(ParticleData) * instanceCount;
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(ParticleData);

			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = particleData.data();
			result = aGfx.pDevice->CreateBuffer(&descGPUBuffer, &initData, &srcParticleCompBuffer);

			// Shader view

			D3D11_BUFFER_DESC descBuf;
			ZeroMemory(&descBuf, sizeof(descBuf));
			srcParticleCompBuffer->GetDesc(&descBuf);

			D3D11_SHADER_RESOURCE_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			descView.BufferEx.FirstElement = 0;

			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
			result = aGfx.pDevice->CreateShaderResourceView(srcParticleCompBuffer, &descView, &srcParticleSRV);
			assert(SUCCEEDED(result));
		}

		/// Output - Particle buffer

		{
			// Buffer in GPU memory

			D3D11_BUFFER_DESC descGPUBuffer;
			ZeroMemory(&descGPUBuffer, sizeof(descGPUBuffer));
			descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS |
				D3D11_BIND_SHADER_RESOURCE;
			descGPUBuffer.ByteWidth = sizeof(ParticleData) * instanceCount;
			descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			descGPUBuffer.StructureByteStride = sizeof(ParticleData);

			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = particleData.data();
			result = aGfx.pDevice->CreateBuffer(&descGPUBuffer, &initData, &destParticleCompBuffer);

			// Unordered access view

			D3D11_BUFFER_DESC descBuf;
			ZeroMemory(&descBuf, sizeof(descBuf));
			destParticleCompBuffer->GetDesc(&descBuf);

			D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			descView.Buffer.FirstElement = 0;

			// Format must be must be DXGI_FORMAT_UNKNOWN, when creating 
			// a View of a Structured Buffer
			descView.Format = DXGI_FORMAT_UNKNOWN;
			descView.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
			result = aGfx.pDevice->CreateUnorderedAccessView(destParticleCompBuffer, &descView, &particleUAV);
			assert(SUCCEEDED(result));
		}
	}

	void Sprite::Draw(Graphics& aGfx)
	{
		texture->Bind(aGfx);

		unsigned int strides[2];
		unsigned int offsets[2];
		ID3D11Buffer* bufferPointers[2];

		strides[0] = sizeof(SpriteVertex);
		strides[1] = sizeof(unsigned int);

		offsets[0] = 0;
		offsets[1] = 0;

		aGfx.pContext->VSSetShaderResources(11u, 1u, &srcTransformSRV);

		bufferPointers[0] = vertexBuffer;
		bufferPointers[1] = instanceBuffer;

		aGfx.pContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

		indexBuffer.Bind(aGfx);
		inputLayout.Bind(aGfx);

		const unsigned int instanceCount = instanceData.size();

		TransformConstantBuffer transformConstantBuffer(aGfx, *this, 0u);
		transformConstantBuffer.Bind(aGfx);

		vertexShader->Bind(aGfx);
		if (aGfx.HasPixelShaderOverride())
		{
			aGfx.GetPixelShaderOverride()->Bind(aGfx);
		}
		else
		{
			pixelShader->Bind(aGfx);
		}

		aGfx.DrawIndexedInstanced(6u, instanceCount);

		ID3D11ShaderResourceView* ppSRVNULL[2] = {NULL, NULL};

		aGfx.pContext->VSSetShaderResources(11u, 2u, ppSRVNULL);

		// Unbind shader resources
		ID3D11ShaderResourceView* nullSRVs[4] = {nullptr};
		aGfx.pContext->PSSetShaderResources(2u, 4u, nullSRVs);
	}

	void Sprite::SetPosition(const DirectX::XMFLOAT3 aPosition, const unsigned int aIndex)
	{
		// Set position directly in transform
		instanceData[aIndex].instanceTransform.r[3] = DirectX::XMVectorSet(aPosition.x, aPosition.y, aPosition.z, 1.0f);
	}

	void Sprite::SetScale(const float aScale, const unsigned int aIndex)
	{
		DirectX::XMFLOAT3 position = GetPosition(aIndex);
		instanceData[aIndex].instanceTransform *= DirectX::XMMatrixScaling(aScale, aScale, aScale);
		SetPosition(position, aIndex);
	}


	DirectX::XMMATRIX Sprite::GetTransform() const
	{
		return DirectX::XMMatrixIdentity();
		//return transforms;
	}

	DirectX::XMFLOAT3 Sprite::GetPosition(unsigned int aIndex) const
	{
		DirectX::XMFLOAT3 position = {
			instanceData[aIndex].instanceTransform.r[3].m128_f32[0],
			instanceData[aIndex].instanceTransform.r[3].m128_f32[1],
			instanceData[aIndex].instanceTransform.r[3].m128_f32[2]
		};
		return position;
	}

	void Sprite::ShowControlWindow(const char* aWindowName)
	{
		//aWindowName = aWindowName ? aWindowName : "Sprite";

		//if (ImGui::Begin(aWindowName))
		//{
		//	ImGui::Text("Orientation");
		//	ImGui::SliderAngle("X", transforms.r[0].m128_f32 + 0, -180.0f, 180.0f);
		//	ImGui::SliderAngle("Y", transforms.r[0].m128_f32 + 1, -180.0f, 180.0f);
		//	ImGui::SliderAngle("Z", transforms.r[0].m128_f32 + 2, -180.0f, 180.0f);
		//	ImGui::Text("Position");
		//	ImGui::DragFloat3("XYZ", transforms.r[3].m128_f32);
		//	ImGui::Text("Scale");
		//	ImGui::DragFloat("XYZ", transforms.r[4].m128_f32, 0.1f, 0.0f, 10.0f, "%.1f");
		//}
		//ImGui::End();
	}

	bool Sprite::IsInSpotlightCone(DirectX::XMFLOAT3 aWorldPosition, const SpotlightData& aSpotlightData)
	{
		const DirectX::XMFLOAT3 toLight = DirectX::XMFLOAT3(aSpotlightData.position.x - aWorldPosition.x, aSpotlightData.position.y - aWorldPosition.y, aSpotlightData.position.z - aWorldPosition.z);
		const float distToLight = DirectX::XMVector3Length(DirectX::XMLoadFloat3(&toLight)).m128_f32[0];
		const DirectX::XMFLOAT3 lightDir = aSpotlightData.direction;
		const float angle = DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&toLight), DirectX::XMLoadFloat3(&lightDir)).m128_f32[0] / distToLight;

		return (angle > cos(aSpotlightData.innerAngle + (aSpotlightData.outerAngle - aSpotlightData.innerAngle) / 2.0f)) && (distToLight < aSpotlightData.range);
	}

	void Sprite::Update(const Graphics& aGfx, const float aDeltaTime, const DirectX::XMFLOAT3 aCameraPosition)
	{
		const unsigned int instanceCount = instanceData.size();

		// Rotate the sprite to face the camera
		const DirectX::XMVECTOR cameraForward = aGfx.camera->GetForwardVector();

		// Get right vector from cross product of forward and up
		DirectX::XMVECTOR cameraRight = DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), cameraForward);
		cameraRight = DirectX::XMVector3Normalize(cameraRight);
		DirectX::XMVECTOR cameraUp = DirectX::XMVector3Cross(cameraForward, cameraRight);
		cameraUp = DirectX::XMVector3Normalize(cameraUp);

		elapsedTime += aDeltaTime;

		particleConstants.elapsedTime = elapsedTime;
		particleConstants.deltaTime = aDeltaTime;
		particleConstants.cameraForward = cameraForward;
		particleConstants.cameraRight = cameraRight;
		particleConstants.cameraUp = cameraUp;
		particleConstants.maxRange = MAX_RANGE_TO_CAMERA;
		particleConstants.cameraPosition = aCameraPosition;

		ComputeConstantBuffer<ParticleConstants> particleConstantBuffer{aGfx, 0u};
		particleConstantBuffer.Update(aGfx, particleConstants);
		particleConstantBuffer.Bind(aGfx);

		ID3D11UnorderedAccessView* ppUAViewNULL[1] = {NULL};
		ID3D11ShaderResourceView* ppSRVNULL[2] = {NULL, NULL};

		computeShader->Bind(aGfx);

		// Transform buffer - Slot 0
		aGfx.pContext->CSSetUnorderedAccessViews(0u, 1u, &instanceUAV,NULL);
		// Particle buffer - Slot 1
		aGfx.pContext->CSSetShaderResources(1u, 1u, &srcParticleSRV);
		aGfx.pContext->CSSetUnorderedAccessViews(1u, 1u, &particleUAV, NULL);

		const unsigned int threadGroupCount = instanceCount / 64;

		aGfx.pContext->Dispatch(threadGroupCount, 1u, 1u);

		aGfx.pContext->CSSetShader(NULL, NULL, 0u);
		aGfx.pContext->CSSetUnorderedAccessViews(0u, 1u, ppUAViewNULL, NULL);
		aGfx.pContext->CSSetShaderResources(0u, 2u, ppSRVNULL);
		aGfx.pContext->CSSetUnorderedAccessViews(1u, 1u, ppUAViewNULL, NULL);
		aGfx.pContext->CSSetShaderResources(1u, 2u, ppSRVNULL);


		//constexpr int threadCount = 6;

		//std::thread threads[threadCount];
		//for (int i = 0; i < threadCount; ++i)
		//{
		//	threads[i] = std::thread(&Sprite::UpdateTransforms, this, aDeltaTime, cameraForward, cameraRight, cameraUp, i * (instanceCount / threadCount), (i + 1) * (instanceCount / threadCount));
		//	threads[i].detach();
		//}

		//for (int i = 0; i < 20; ++i)
		//{
		//	threads[i].join();
		//}
		//updateCounter += updateIncrease;

		//if (updateCounter >= instanceCount)
		//{
		//	updateCounter = 0;
		//	updateIndex = 0;
		//}
	}
}
