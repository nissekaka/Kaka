#include "stdafx.h"
#include "ModelRenderer.h"

#include <ranges>

#include "Model.h"

namespace Kaka
{

	void ModelRenderer::Init(const Graphics& aGfx)
	{
		topology.Init(aGfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void ModelRenderer::BuildRenderQueue(const Graphics& aGfx, RenderQueue& aRenderQueue, const std::vector<RenderData>& aRenderData)
	{
		aRenderQueue.commands.clear();

		std::unordered_map<std::string, RenderQueue::RenderCommand> groups;

		for (const RenderData& package : aRenderData)
		{
			std::string key = std::to_string(static_cast<int>(package.vertexShader->GetType())) + "|" +
				std::to_string(static_cast<int>(package.pixelShader->GetType())) + "|" +
				package.filePath;

			RenderQueue::RenderCommand& renderCommand = groups[key];
			renderCommand.vertexShader = package.vertexShader;
			renderCommand.pixelShader = package.pixelShader;
			renderCommand.meshList = package.meshList;
			renderCommand.instanceTransforms.push_back(package.transform);
		}

		for (RenderQueue::RenderCommand& command : groups | std::views::values)
		{
			std::vector<DirectX::XMMATRIX> instanceData;
			instanceData.reserve(command.instanceTransforms.size());

			for (DirectX::XMMATRIX* matrixPtr : command.instanceTransforms)
			{
				instanceData.push_back(*matrixPtr);
			}

			command.instanceBuffer.Init(aGfx, instanceData);

			aRenderQueue.commands.push_back(command);
		}
	}

	void ModelRenderer::DrawRenderQueue(Graphics& aGfx, RenderQueue& aRenderQueue, const bool aShadowPass)
	{
		Texture* boundTexture = nullptr;

		topology.Bind(aGfx);

		for (RenderQueue::RenderCommand& command : aRenderQueue.commands)
		{
			// Commands SHOULD be sorted by shader, so we can optimize by binding the shader only once
			// This is a naive implementation, but it works for now
			if (aGfx.HasVertexShaderOverride())
			{
				if (currentVertexShader != aGfx.GetVertexShaderOverride())
				{
					aGfx.GetVertexShaderOverride()->Bind(aGfx);
					currentVertexShader = aGfx.GetVertexShaderOverride();
				}
			}
			else if (currentVertexShader != command.vertexShader)
			{
				command.vertexShader->Bind(aGfx);
				currentVertexShader = command.vertexShader;
			}

			if (aGfx.HasPixelShaderOverride())
			{
				if (currentPixelShader != aGfx.GetPixelShaderOverride())
				{
					aGfx.GetPixelShaderOverride()->Bind(aGfx);
					currentPixelShader = aGfx.GetPixelShaderOverride();
				}
			}
			else if (currentPixelShader != command.pixelShader)
			{
				command.pixelShader->Bind(aGfx);
				currentPixelShader = command.pixelShader;
			}

			std::vector<DirectX::XMMATRIX> instanceData;
			instanceData.reserve(command.instanceTransforms.size());

			for (DirectX::XMMATRIX* matrixPtr : command.instanceTransforms)
			{
				AABB aabb = Model::GetTranslatedAABB(command.meshList->meshes[0], *matrixPtr);

				if (aGfx.IsBoundingBoxInFrustum(aabb.minBound, aabb.maxBound))
				{
					DirectX::XMMATRIX transform = *matrixPtr;
					if (aShadowPass)
					{
						DirectX::XMMATRIX objectToClip = transform * aGfx.GetCameraInverseView();
						objectToClip = objectToClip * aGfx.GetProjection();
						transform = objectToClip;
					}

					instanceData.push_back(transform);
				}
			}

			//std::vector<DirectX::XMMATRIX> instanceData;
			//instanceData.reserve(instanceDataPreCulling.size());

			//for (DirectX::XMMATRIX transform : instanceDataPreCulling)
			//{
			//	if (aShadowPass)
			//	{
			//		DirectX::XMMATRIX objectToClip = transform * aGfx.GetCameraInverseView();
			//		objectToClip = objectToClip * aGfx.GetProjection();
			//		transform = objectToClip;
			//	}

			//	instanceData.push_back(transform); // Dereference the pointer and copy the matrix
			//}

			//std::vector<DirectX::XMMATRIX> instanceData;
			//instanceData.reserve(command.instanceTransforms.size());

			//for (DirectX::XMMATRIX* matrixPtr : command.instanceTransforms)
			//{
			//	DirectX::XMMATRIX transform = *matrixPtr;
			//	if (aShadowPass)
			//	{
			//		DirectX::XMMATRIX objectToClip = transform * aGfx.GetCameraInverseView();
			//		objectToClip = objectToClip * aGfx.GetProjection();
			//		transform = objectToClip;
			//	}

			//	instanceData.push_back(transform); // Dereference the pointer and copy the matrix
			//}

			// Update data in instance buffer
			command.instanceBuffer.Update(aGfx, instanceData);
			command.instanceBuffer.Bind(aGfx);

			MeshList& meshList = *command.meshList;

			for (Mesh& mesh : meshList.meshes)
			{
				if (mesh.texture != nullptr)
				{
					if (boundTexture == nullptr || mesh.texture != boundTexture)
					{
						mesh.texture->Bind(aGfx);

						if (mesh.texture->HasAlpha())
						{
							aGfx.SetRasterizerState(eRasterizerStates::NoCulling);
						}
					}
				}

				mesh.vertexBuffer.Bind(aGfx);
				mesh.indexBuffer.Bind(aGfx);

				if (mesh.texture->HasAlpha())
				{
					aGfx.SetRasterizerState(eRasterizerStates::BackfaceCulling);
				}

				aGfx.DrawIndexedInstanced(mesh.indexBuffer.GetCount(), instanceData.size());
			}

			// Unbind shader resources
			ID3D11ShaderResourceView* nullSRVs[3] = { nullptr };
			aGfx.pContext->PSSetShaderResources(1u, 3u, nullSRVs);
		}
	}
}
