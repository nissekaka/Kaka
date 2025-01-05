#include "stdafx.h"
//#include "ModelRenderer.h"
//
//#include "ModelLoader.h"
//#include "ECS/ECS.h"
//
//namespace Kaka
//{
//	ModelRenderer::ModelRenderer()
//	{
//		renderQueue.reserve(1024);
//	}
//
//	void ModelRenderer::RegisterEntity(const EntityID aEntity)
//	{
//		renderQueue.push_back(aEntity);
//	}
//
//	void ModelRenderer::DeregisterEntity(const EntityID aEntity)
//	{
//		std::erase(renderQueue, aEntity);
//	}
//
//	void ModelRenderer::LoadModel(const Graphics& aGfx, const std::string& aFilePath)
//	{
//		if (ModelLoader::LoadStaticModel(aGfx, aFilePath, modelData.emplace_back()))
//		{
//			// TODO Shader should be set from Editor or something
//			modelData.back().vertexShader = ShaderFactory::GetVertexShader(aGfx, eVertexShaderType::ModelTAA);
//			modelData.back().pixelShader = ShaderFactory::GetPixelShader(aGfx, ePixelShaderType::Model);
//		}
//		else
//		{
//			modelData.pop_back();
//		}
//	}
//
//	void ModelRenderer::RenderQueue(const Graphics& aGfx)
//	{
//		for (const auto& entity : renderQueue)
//		{
//			void RenderModel(Graphics & aGfx, ModelComponent & model, TransformComponent & transform)
//			{
//				// TODO Figure out a better way to handle transforms
//				const DirectX::XMMATRIX objectToWorld = DirectX::XMMatrixScaling(transform.scale, transform.scale, transform.scale) *
//					DirectX::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
//					DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);;
//				DirectX::XMMATRIX objectToClip = objectToWorld * aGfx.GetCameraInverseView();
//				objectToClip = objectToClip * aGfx.GetJitteredProjection();
//
//				transform.transforms = { objectToWorld, objectToClip };
//
//				VertexConstantBuffer<Transforms> transformBuffer = { aGfx, transform.transforms, 0u };
//				transformBuffer.Update(aGfx, transform.transforms);
//				transformBuffer.Bind(aGfx);
//
//				MeshList& meshList = ModelLoader::GetMeshList(model.ModelComponent::filePath);
//
//				for (Mesh& mesh : meshList.meshes)
//				{
//					//if (aFrustumCulling)
//					//{
//					if (!aGfx.IsBoundingBoxInFrustum(Model::GetTranslatedAABB(mesh, objectToWorld).minBound, Model::GetTranslatedAABB(mesh, objectToWorld).maxBound))
//					{
//						continue;
//					}
//					//}
//
//					bool hasAlpha = false;
//					if (mesh.texture != nullptr)
//					{
//						mesh.texture->Bind(aGfx);
//
//						if (mesh.texture->HasAlpha())
//						{
//							hasAlpha = true;
//							aGfx.SetRasterizerState(eRasterizerStates::NoCulling);
//						}
//					}
//
//					mesh.vertexBuffer.Bind(aGfx);
//					mesh.indexBuffer.Bind(aGfx);
//
//					aGfx.DrawIndexed(mesh.indexBuffer.GetCount());
//
//					if (hasAlpha)
//					{
//						aGfx.SetRasterizerState(eRasterizerStates::BackfaceCulling);
//					}
//
//					//if (aDrawDebug)
//					//{
//					//	DrawDebugAABB(aGfx, mesh, objectToWorld);
//					//}
//				}
//
//				// Unbind shader resources
//				ID3D11ShaderResourceView* nullSRVs[3] = { nullptr };
//				aGfx.pContext->PSSetShaderResources(1u, 3u, nullSRVs);
//			}
//		}
//	}
//}
