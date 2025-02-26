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
		groups.clear();
		commandInstanceData.clear();

		for (RenderData& renderData : aRenderData)
		{
			uint64_t key = GetRenderDataHash(std::to_string(static_cast<int>(renderData.vertexShader->GetType())),
					std::to_string(static_cast<int>(renderData.pixelShader->GetType())),
					renderData.modelData->filePath);

			RenderQueue::RenderCommand& renderCommand = groups[key];
			renderCommand.vertexShader = renderData.vertexShader;
			renderCommand.pixelShader = renderData.pixelShader;
			renderCommand.modelData = renderData.modelData;
			renderCommand.transformComponents.push_back(renderData.transform);
		}

		for (RenderQueue::RenderCommand& command : groups | std::views::values)
		{
			commandInstanceData.clear();
			commandInstanceData.reserve(command.transformComponents.size());

			for (const Ecs::TransformComponent* transform : command.transformComponents)
			{
				commandInstanceData.push_back(CreateTransformMatrix(transform));
			}

			command.instanceBuffer.Reset();
			command.instanceBuffer.Init(aGfx, commandInstanceData);

			aRenderQueue.commands.push_back(command);
		}
	}

	void ModelRenderer::DrawRenderQueue(Graphics& aGfx, RenderQueue& aRenderQueue, const bool aShadowPass)
	{
		boundTexture = nullptr;
		topology.Bind(aGfx);
		renderInstanceData.clear();

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

			for (Ecs::TransformComponent* transform : command.transformComponents)
			{
				DirectX::XMMATRIX objectToWorld = CreateTransformMatrix(transform);

				AABB aabb = GetTranslatedAABB(command.modelData->aabb, objectToWorld);

				if (aGfx.IsBoundingBoxInFrustum(aabb))
				{
					if (aShadowPass)
					{
						DirectX::XMMATRIX objectToClip = objectToWorld * aGfx.GetCameraInverseView();
						objectToClip = objectToClip * aGfx.GetProjection();
						objectToWorld = objectToClip;
					}
					else
					{
						aGfx.DrawDebugAABB(command.modelData->aabb, objectToWorld);
					}

					renderInstanceData.push_back(objectToWorld);
				}
			}

			if (renderInstanceData.empty())
			{
				continue;
			}

			// Update data in instance buffer
			command.instanceBuffer.Update(aGfx, renderInstanceData);
			command.instanceBuffer.Bind(aGfx);

			ModelData& modelData = *command.modelData;

			for (Mesh& mesh : modelData.meshes)
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

				aGfx.DrawIndexedInstanced(mesh.indexBuffer.GetCount(), renderInstanceData.size());
			}

			// Unbind shader resources
			ID3D11ShaderResourceView* nullSRVs[3] = { nullptr };
			aGfx.pContext->PSSetShaderResources(1u, 3u, nullSRVs);

			renderInstanceData.clear();
		}
	}

	DirectX::XMMATRIX ModelRenderer::CreateTransformMatrix(const Ecs::TransformComponent* aTransform)
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

	AABB ModelRenderer::GetTranslatedAABB(const AABB& aAabb, const DirectX::XMMATRIX& aTransform)
	{
		const DirectX::XMMATRIX meshTransform = aTransform;
		const DirectX::XMVECTOR minBound = DirectX::XMVector3Transform(
			DirectX::XMLoadFloat3(&aAabb.minBound),
			meshTransform
		);
		const DirectX::XMVECTOR maxBound = DirectX::XMVector3Transform(
			DirectX::XMLoadFloat3(&aAabb.maxBound),
			meshTransform
		);
		AABB aabb;
		DirectX::XMStoreFloat3(&aabb.minBound, minBound);
		DirectX::XMStoreFloat3(&aabb.maxBound, maxBound);
		return aabb;
	}

	uint64_t ModelRenderer::GetRenderDataHash(const std::string& aVertexShader, const std::string& aPixelShader, const std::string& aFilePath)
	{
		constexpr std::hash<std::string> hasher;
		return hasher(aVertexShader + aPixelShader + aFilePath);
	}
}
