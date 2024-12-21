#include "stdafx.h"
#include "PostProcessing.h"

#include "Core/Graphics/Bindable/PixelShader.h"
#include "Core/Graphics/Bindable/VertexShader.h"
#include "Core/Graphics/Drawable/ModelData.h"
#include "Core/Graphics/Drawable/Vertex.h"

namespace Kaka
{
	PostProcessing::PostProcessing() {}

	void PostProcessing::Init(const Graphics& aGfx)
	{
		postProcessVS = ShaderFactory::GetVertexShader(aGfx, L"Shaders\\Fullscreen_VS.cso");
		postProcessPS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\PostProcessing_PS.cso");

		downsamplePS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Downsample_PS.cso");
		upsamplePS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Upsample_PS.cso");
		fullscreenPS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Fullscreen_PS.cso");
		temporalAliasingPS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\TAA_PS.cso");

		currentPS = postProcessPS;

		struct PVertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 tex;
		};

		PVertex _vertices[4] = {
			{
				{-1.0f, -1.0f, 0.0f},
				{0, 1},
			},
			{
				{-1.0f, 1.0f, 0.0f},
				{0, 0},
			},
			{
				{1.0f, -1.0f, 0.0f},
				{1, 1},
			},
			{
				{1.0f, 1.0f, 0.0f},
				{1, 0},
			}
		};

		unsigned short _indices[6] = {0, 1, 2, 2, 1, 3};

		std::vector<PVertex> vertices;
		std::vector<unsigned short> indices;

		for (int i = 0; i < 4; i++)
		{
			vertices.push_back(_vertices[i]);
		}

		for (int i = 0; i < 6; i++)
		{
			indices.push_back(_indices[i]);
		}

		vertexBuffer.Init(aGfx, vertices);
		indexBuffer.Init(aGfx, indices);

		ied =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
		};

		inputLayout.Init(aGfx, ied, postProcessVS->GetBytecode());
		topology.Init(aGfx, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void PostProcessing::Draw(Graphics& aGfx)
	{
		vertexBuffer.Bind(aGfx);
		indexBuffer.Bind(aGfx);
		currentPS->Bind(aGfx);
		postProcessVS->Bind(aGfx);
		inputLayout.Bind(aGfx);
		topology.Bind(aGfx);

		aGfx.DrawIndexed(6u);
		// Unbind shader resources
		ID3D11ShaderResourceView* nullSRVs[1u] = {nullptr};
		aGfx.pContext->PSSetShaderResources(0u, 1u, nullSRVs);
	}

	void PostProcessing::SetDownsamplePS()
	{
		currentPS = downsamplePS;
	}

	void PostProcessing::SetUpsamplePS()
	{
		currentPS = upsamplePS;
	}

	void PostProcessing::SetPostProcessPS()
	{
		currentPS = postProcessPS;
	}

	void PostProcessing::SetFullscreenPS()
	{
		currentPS = fullscreenPS;
	}

	void PostProcessing::SetTemporalAliasingPS()
	{
		currentPS = temporalAliasingPS;
	}
}
