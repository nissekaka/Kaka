#pragma once
#include "Drawable.h"
#include "Core/Graphics/Graphics.h"
#include "Core/Graphics/Bindable/BindableCommon.h"

namespace Kaka
{
	struct Vertex;

	class ReflectionPlane : public Drawable
	{
	public:
		void Init(const Graphics& aGfx, float aSize);
		void Draw(Graphics& aGfx);
		void SetPosition(DirectX::XMFLOAT3 aPosition);
		void SetScale(float aScale);
		DirectX::XMMATRIX GetTransform() const override;
		DirectX::XMFLOAT3 GetPosition() const;

	public:
		void ShowControlWindow(const char* aWindowName = nullptr);

	private:
		Sampler sampler = {};
		PixelShader* pixelShader = nullptr;
		VertexShader* vertexShader = nullptr;
		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
		};
		InputLayout inputLayout;
		Topology topology = {};
		Rasterizer rasterizer = {};
		DepthStencil depthStencil = {};

	private:
		std::vector<Vertex> vertices = {};
		std::vector<unsigned short> indices = {};
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

	private:
		struct TransformParameters
		{
			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			float scale = 1.0f;
		};

		TransformParameters transform;

		struct PSMaterialConstant
		{
			BOOL normalMapEnabled = TRUE;
			BOOL materialEnabled = TRUE;
			float padding[2] = {};
		};

		PSMaterialConstant pmc;

		Texture texture{3u};
	};
}
