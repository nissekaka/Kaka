#pragma once
#include "Core/Graphics/Drawable/Drawable.h"
#include "Core/Graphics/Bindable/CubeTexture.h"

namespace Kaka
{
	class Skybox : public Drawable
	{
	public:
		Skybox() = default;
		~Skybox() override = default;
		void Init(const Graphics& aGfx, const std::string& aDayPath, const std::string& aNightPath);
		void Draw(Graphics& aGfx);
		void Rotate(DirectX::XMFLOAT3 aRotation);
		void FlipScale();

	public:
		DirectX::XMMATRIX GetTransform() const override;

	private:
		struct TransformParameters
		{
			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			DirectX::XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
		};

		TransformParameters transform;

		struct Transforms
		{
			DirectX::XMMATRIX projection;
		};

		//Sampler sampler = {};
		VertexBuffer vertexBuffer = {};
		IndexBuffer indexBuffer = {};
		PixelShader* pixelShader = nullptr;
		VertexShader* vertexShader = nullptr;
		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
		};
		InputLayout inputLayout = {};
		Topology topology = {};

		struct Vertex
		{
			struct
			{
				float x;
				float y;
				float z;
			} pos;
		};

		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f, -0.5f}},
			{{0.5f, -0.5f, -0.5f}},
			{{-0.5f, 0.5f, -0.5f}},
			{{0.5f, 0.5f, -0.5f}},
			{{-0.5f, -0.5f, 0.5f}},
			{{0.5f, -0.5f, 0.5f}},
			{{-0.5f, 0.5f, 0.5f}},
			{{0.5f, 0.5f, 0.5f}},
		};

		const std::vector<unsigned short> indices =
		{
			0, 2, 1, 2, 3, 1,
			1, 3, 5, 3, 7, 5,
			2, 6, 3, 3, 6, 7,
			4, 5, 7, 4, 7, 6,
			0, 4, 2, 2, 4, 6,
			0, 1, 4, 1, 5, 4
		};

	private:
		CubeTexture dayTexture{12u};
		CubeTexture nightTexture{13u};
	};
}
