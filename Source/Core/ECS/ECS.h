#pragma once
#include <ranges>

#include "Core/Graphics/Bindable/ConstantBuffer.h"
#include "Core/Graphics/Graphics.h"
#include "Graphics/Drawable/ModelLoader.h"
#include "imgui/imgui.h"
//#include "Graphics/Drawable/Model.h"

namespace Kaka
{
	using EntityID = int64_t;

	template <typename T>
	using ComponentMap = std::unordered_map<EntityID, T>;

	struct Transforms
	{
		DirectX::XMMATRIX objectToWorld = {};
		DirectX::XMMATRIX objectToClip = {};
	};

	struct TransformComponent
	{
		//DirectX::XMMATRIX transform;
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float scale = 1.0f;

		Transforms transforms = {};
	};

	struct ModelComponent
	{
		std::string filePath = "";
	};

	class ECS
	{
	public:
		using TransformComponents = ComponentMap<TransformComponent>;
		using ModelComponents = ComponentMap<ModelComponent>;

		struct Components
		{
			TransformComponents transformComponents;
			ModelComponents modelComponents;
		} components;

		void UpdateTransformComponents(TransformComponents& aTransforms)
		{
			// TODO This is currently not necessary because we're not updating the transform components
			// The data should be stored in the component, but it can't be manipulated from anywhere currently
			for (const auto& entity : aTransforms | std::views::keys)
			{
				aTransforms[entity].scale = 0.1f;
			}
		}

		void RenderModelComponents(Graphics& aGfx, ModelComponents& aModels, TransformComponents& aTransforms, bool drawDebug = false)
		{
			for (auto& [entity, model] : aModels)
			{
				auto& transform = aTransforms[entity];

				// TODO Figure out a better way to handle transforms
				const DirectX::XMMATRIX objectToWorld = DirectX::XMMatrixScaling(transform.scale, transform.scale, transform.scale) *
					DirectX::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
					DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);;
				DirectX::XMMATRIX objectToClip = objectToWorld * aGfx.GetCameraInverseView();
				objectToClip = objectToClip * aGfx.GetJitteredProjection();

				transform.transforms = { objectToWorld, objectToClip };

				VertexConstantBuffer<Transforms> transformBuffer = { aGfx, transform.transforms, 0u };
				transformBuffer.Update(aGfx, transform.transforms);
				transformBuffer.Bind(aGfx);

				MeshList& meshList = ModelLoader::GetMeshList(model.filePath);

				for (Mesh& mesh : meshList.meshes)
				{
					//if (aFrustumCulling)
					//{
					if (!aGfx.IsBoundingBoxInFrustum(Model::GetTranslatedAABB(mesh, objectToWorld).minBound, Model::GetTranslatedAABB(mesh, objectToWorld).maxBound))
					{
						continue;
					}
					//}

					bool hasAlpha = false;
					if (mesh.texture != nullptr)
					{
						mesh.texture->Bind(aGfx);

						if (mesh.texture->HasAlpha())
						{
							hasAlpha = true;
							aGfx.SetRasterizerState(eRasterizerStates::NoCulling);
						}
					}

					mesh.vertexBuffer.Bind(aGfx);
					mesh.indexBuffer.Bind(aGfx);

					aGfx.DrawIndexed(mesh.indexBuffer.GetCount());

					if (hasAlpha)
					{
						aGfx.SetRasterizerState(eRasterizerStates::BackfaceCulling);
					}

					if (drawDebug)
					{
						DrawDebugAABB(aGfx, mesh, objectToWorld);
					}
				}

				// Unbind shader resources
				ID3D11ShaderResourceView* nullSRVs[3] = { nullptr };
				aGfx.pContext->PSSetShaderResources(1u, 3u, nullSRVs);
			}
		}

		EntityID CreateEntity()
		{
			entities++;
			return entities;
		}

	private:
		//VertexConstantBuffer <Transforms> transformBuffer = { 0u };
		EntityID entities = 0;

		// TODO Move this to a more appropriate place
		static inline void DrawDebugAABB(const Graphics& aGfx, const Mesh& aMesh, const DirectX::XMMATRIX& aTransform)
		{
			struct Cube
			{
				DirectX::XMFLOAT3 vertices[8];
			};

			const AABB aabb = aMesh.aabb;

			Cube cube;
			cube.vertices[0] = { aabb.minBound.x, aabb.minBound.y, aabb.minBound.z };
			cube.vertices[1] = { aabb.minBound.x, aabb.maxBound.y, aabb.minBound.z };
			cube.vertices[2] = { aabb.maxBound.x, aabb.maxBound.y, aabb.minBound.z };
			cube.vertices[3] = { aabb.maxBound.x, aabb.minBound.y, aabb.minBound.z };
			cube.vertices[4] = { aabb.minBound.x, aabb.minBound.y, aabb.maxBound.z };
			cube.vertices[5] = { aabb.minBound.x, aabb.maxBound.y, aabb.maxBound.z };
			cube.vertices[6] = { aabb.maxBound.x, aabb.maxBound.y, aabb.maxBound.z };
			cube.vertices[7] = { aabb.maxBound.x, aabb.minBound.y, aabb.maxBound.z };

			// If any vertex is outside of camera frustum, don't draw
			for (auto& vertice : cube.vertices)
			{
				DirectX::XMVECTOR transformedVertex = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&vertice), aTransform);
				DirectX::XMFLOAT3 transformedVertexFloat3;
				DirectX::XMStoreFloat3(&transformedVertexFloat3, transformedVertex);

				if (!aGfx.IsPointInFrustum(transformedVertexFloat3))
				{
					return;
				}
			}

			DirectX::XMFLOAT2 screenPos[8];

			// Convert 3D positions to screen space
			for (int i = 0; i < 8; ++i)
			{
				DirectX::XMMATRIX projectionMatrix = aTransform * aGfx.GetCameraInverseView();
				projectionMatrix = projectionMatrix * aGfx.GetProjection();

				DirectX::XMStoreFloat2(
					&screenPos[i],
					DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&cube.vertices[i]), projectionMatrix)
				);

				screenPos[i].x = (screenPos[i].x + 1.0f) * 0.5f * ImGui::GetIO().DisplaySize.x;
				screenPos[i].y = (1.0f - screenPos[i].y) * 0.5f * ImGui::GetIO().DisplaySize.y;
			}

			// Draw lines between all screenpositions
			for (int i = 0; i < 4; ++i)
			{
				ImGui::GetForegroundDrawList()->AddLine(
					ImVec2(screenPos[i].x, screenPos[i].y),
					ImVec2(screenPos[(i + 1) % 4].x, screenPos[(i + 1) % 4].y),
					IM_COL32(0, 255, 0, 255)
				);
				ImGui::GetForegroundDrawList()->AddLine(
					ImVec2(screenPos[i + 4].x, screenPos[i + 4].y),
					ImVec2(screenPos[((i + 1) % 4) + 4].x, screenPos[((i + 1) % 4) + 4].y),
					IM_COL32(0, 255, 0, 255)
				);
				ImGui::GetForegroundDrawList()->AddLine(
					ImVec2(screenPos[i].x, screenPos[i].y),
					ImVec2(screenPos[i + 4].x, screenPos[i + 4].y),
					IM_COL32(0, 255, 0, 255)
				);
			}
		}
	};
}
