#include "stdafx.h"
#include "ModelRenderer.h"

#include <ranges>

#include "Model.h"
#include "ECS/Components/TransformComponent.h"

namespace Kaka
{

	void ModelRenderer::Init(const Graphics& aGfx)
	{
		topology.Init(aGfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void ModelRenderer::BuildRenderQueue(const Graphics& aGfx, RenderQueue& aRenderQueue, std::vector<RenderData>& aRenderData)
	{
		aRenderQueue.commands.clear();

		std::unordered_map<std::string, RenderQueue::RenderCommand> groups;

		for (RenderData& package : aRenderData)
		{
			std::string key = std::to_string(static_cast<int>(package.vertexShader->GetType())) + "|" +
				std::to_string(static_cast<int>(package.pixelShader->GetType())) + "|" +
				package.meshList->filePath;

			RenderQueue::RenderCommand& renderCommand = groups[key];
			renderCommand.vertexShader = package.vertexShader;
			renderCommand.pixelShader = package.pixelShader;
			renderCommand.meshList = package.meshList;
			renderCommand.transformComponents.push_back(package.transform);
		}

		for (RenderQueue::RenderCommand& command : groups | std::views::values)
		{
			std::vector<DirectX::XMMATRIX> instanceData;
			instanceData.reserve(command.transformComponents.size());

			for (TransformComponent* transform : command.transformComponents)
			{
				instanceData.push_back(CreateTransformMatrix(transform));
			}

			command.instanceBuffer.Reset();
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
			// TODO Commands SHOULD be sorted by shader, so we can optimize by binding the shader only once
			// TODO This is a naive implementation, but it works for now
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

			for (TransformComponent* transform : command.transformComponents)
			{
				DirectX::XMMATRIX objectToWorld = CreateTransformMatrix(transform);

				AABB aabb = Model::GetTranslatedAABB(command.meshList->meshes[0], objectToWorld);

				if (aGfx.IsBoundingBoxInFrustum(aabb.minBound, aabb.maxBound))
				{
					if (aShadowPass)
					{
						DirectX::XMMATRIX objectToClip = objectToWorld * aGfx.GetCameraInverseView();
						objectToClip = objectToClip * aGfx.GetProjection();
						objectToWorld = objectToClip;
					}

					instanceData.push_back(objectToWorld);
				}
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

				aGfx.DrawIndexedInstanced(mesh.indexBuffer.GetCount(), instanceData.size());
			}

			// Unbind shader resources
			ID3D11ShaderResourceView* nullSRVs[3] = { nullptr };
			aGfx.pContext->PSSetShaderResources(1u, 3u, nullSRVs);
		}

	}

	DirectX::XMMATRIX ModelRenderer::CreateTransformMatrix(const TransformComponent* aTransform)
	{
		using namespace DirectX;
		XMVECTOR position = XMLoadFloat3(&aTransform->GetPosition());
		XMVECTOR rotation = XMLoadFloat4(&aTransform->GetQuaternionRotation());
		XMVECTOR scale = XMLoadFloat3(&aTransform->GetScale());

		XMMATRIX translationMatrix = XMMatrixTranslationFromVector(position);
		XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotation);
		XMMATRIX scaleMatrix = XMMatrixScalingFromVector(scale);

		return scaleMatrix * rotationMatrix * translationMatrix;
	}
}
