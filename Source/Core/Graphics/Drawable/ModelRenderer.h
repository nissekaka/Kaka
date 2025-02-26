#pragma once
#include "Graphics/Bindable/InstanceBuffer.h"

namespace Kaka
{
	namespace Ecs
	{
		struct TransformComponent;
	}

	struct RenderPackage;
	struct ModelComponent;

	struct RenderQueue
	{
		struct RenderCommand
		{
			ModelData* modelData;
			VertexShader* vertexShader;
			PixelShader* pixelShader;
			std::vector<Ecs::TransformComponent*> transformComponents;
			InstanceBuffer<DirectX::XMMATRIX> instanceBuffer = { 11u };
		};

		std::vector<RenderCommand> commands;
	};

	class ModelRenderer
	{
	public:
		ModelRenderer() = default;
		~ModelRenderer() = default;
		void Init(const Graphics& aGfx);
		void BuildRenderQueue(const Graphics& aGfx, RenderQueue& aRenderQueue, std::vector<RenderData>& aRenderData);
		void DrawRenderQueue(Graphics& aGfx, RenderQueue& aRenderQueue, const bool aShadowPass = false);
		static DirectX::XMMATRIX CreateTransformMatrix(const Ecs::TransformComponent* aTransform);
	private:
		static AABB GetTranslatedAABB(const AABB& aAabb, const DirectX::XMMATRIX& aTransform);
		static uint64_t GetRenderDataHash(const std::string& aVertexShader, const std::string& aPixelShader, const std::string& aFilePath);

	private:
		Topology topology = {};

		VertexShader* currentVertexShader = nullptr;
		PixelShader* currentPixelShader = nullptr;
		Texture* boundTexture = nullptr;

		std::unordered_map<uint64_t, RenderQueue::RenderCommand> groups = {};
		std::vector<DirectX::XMMATRIX> commandInstanceData = {};
		std::vector<DirectX::XMMATRIX> renderInstanceData = {};
	};
}
