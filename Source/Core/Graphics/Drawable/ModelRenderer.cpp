#include "stdafx.h"
#include "ModelRenderer.h"

#include <ranges>

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

		for (const auto& package : aRenderData)
		{
			std::string key = std::to_string(static_cast<int>(package.vertexShader->VertexShader::GetType())) + "|" +
				std::to_string(static_cast<int>(package.pixelShader->PixelShader::GetType())) + "|" +
				package.filePath;

			// Find or create the group
			auto& renderCommand = groups[key];
			renderCommand.vertexShader = package.vertexShader;
			renderCommand.pixelShader = package.pixelShader;
			renderCommand.meshList = package.meshList;

			// Append the transform for instancing
			renderCommand.instanceTransforms.push_back(package.transform);
		}

		// Populate the render queue
		for (auto& command : groups | std::views::values)
		{
			// Create instance data buffer
			std::vector<DirectX::XMMATRIX> instanceData;
			instanceData.reserve(command.instanceTransforms.size());

			for (auto* matrixPtr : command.instanceTransforms)
			{
				instanceData.push_back(DirectX::XMMatrixTranspose(*matrixPtr)); // Transpose for HLSL
			}

			command.instanceBuffer.Init(aGfx, instanceData);

			aRenderQueue.commands.push_back(command);
		}
	}

	void ModelRenderer::DrawRenderQueue(Graphics& aGfx, RenderQueue& aRenderQueue, const bool aShadowPass)
	{
		Texture* boundTexture = nullptr;

		topology.Bind(aGfx);

		for (auto& command : aRenderQueue.commands)
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

			for (auto* matrixPtr : command.instanceTransforms)
			{
				DirectX::XMMATRIX transform = *matrixPtr;
				if (aShadowPass)
				{
					DirectX::XMMATRIX objectToClip = transform * aGfx.GetCameraInverseView();
					objectToClip = objectToClip * aGfx.GetProjection();
					transform = objectToClip;
				}

				instanceData.push_back(transform); // Dereference the pointer and copy the matrix
			}

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

				aGfx.DrawIndexedInstanced(mesh.indexBuffer.GetCount(), command.instanceTransforms.size());
			}

			// Unbind shader resources
			ID3D11ShaderResourceView* nullSRVs[3] = { nullptr };
			aGfx.pContext->PSSetShaderResources(1u, 3u, nullSRVs);
		}
	}
}
