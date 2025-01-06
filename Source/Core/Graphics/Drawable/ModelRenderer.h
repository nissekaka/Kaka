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
			std::vector<DirectX::XMMATRIX*> instanceTransforms;
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
		void BuildRenderQueue(const Graphics& aGfx, RenderQueue& aRenderQueue, const std::vector<RenderData>& aRenderPackages);
		void DrawRenderQueue(Graphics& aGfx, RenderQueue& aRenderQueue, const bool aShadowPass = false);

	private:
		Topology topology = {};

		VertexShader* currentVertexShader = nullptr;
		PixelShader* currentPixelShader = nullptr;
	};
}
