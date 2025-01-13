#pragma once
#include "Graphics/Bindable/InstanceBuffer.h"

namespace Kaka
{
	struct RenderPackage;
	struct TransformComponent;
	struct ModelComponent;

	struct RenderQueue
	{
		struct RenderCommand
		{
			MeshList* meshList;
			VertexShader* vertexShader;
			PixelShader* pixelShader;
			// TODO This vector could cause problems when resized or if transforms are moved in memory
			//std::vector<DirectX::XMMATRIX> instanceTransforms;
			std::vector<TransformComponent*> transformComponents;
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
		static DirectX::XMMATRIX CreateTransformMatrix(const TransformComponent* aTransform);

	private:
		Topology topology = {};

		VertexShader* currentVertexShader = nullptr;
		PixelShader* currentPixelShader = nullptr;
	};
}
