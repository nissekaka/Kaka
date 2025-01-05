//#pragma once
//#include <ECS/Entity.h>
//
//namespace Kaka
//{
//	struct TransformComponent;
//	struct ModelComponent;
//
//	struct RenderRequest
//	{
//		ModelComponent* modelComponent;
//		TransformComponent* transformComponent;
//	};
//
//	class ModelRenderer
//	{
//	public:
//		ModelRenderer();
//		~ModelRenderer() = default;
//		void RegisterEntity(const EntityID aEntity);
//		void DeregisterEntity(const EntityID aEntity);
//		void LoadModel(const Graphics& aGfx, const std::string& aFilePath);
//		void RenderQueue(const Graphics& aGfx);
//
//	private:
//		std::vector<ModelData> modelData = {};
//		std::vector<RenderRequest> renderQueue = {};
//	};
//}