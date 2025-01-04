#include "stdafx.h"
#include "IndirectLighting.h"

namespace Kaka
{
	void IndirectLighting::Init(const Graphics& aGfx)
	{
		indirectLightingVS = ShaderFactory::GetVertexShader(aGfx, eVertexShaderType::Fullscreen);
		indirectLightingPS = ShaderFactory::GetPixelShader(aGfx, ePixelShaderType::IndirectLighting);
		indirectLightingCombinedPS = ShaderFactory::GetPixelShader(aGfx, ePixelShaderType::IndirectLightingCombined);
		indirectLightingDefaultPS = indirectLightingPS;

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

		topology.Init(aGfx, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void IndirectLighting::Draw(Graphics& aGfx)
	{
		vertexBuffer.Bind(aGfx);
		indexBuffer.Bind(aGfx);
		indirectLightingVS->Bind(aGfx);
		indirectLightingPS->Bind(aGfx);

		aGfx.DrawIndexed(indexBuffer.GetCount());
	}

	void IndirectLighting::SetPixelShaderCombined(const bool aValue)
	{
		if (aValue)
		{
			indirectLightingPS = indirectLightingCombinedPS;
		}
		else
		{
			indirectLightingPS = indirectLightingDefaultPS;
		}
	}
}
