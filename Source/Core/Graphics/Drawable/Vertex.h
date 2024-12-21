#pragma once
#include <DirectXMath.h>
#include <array>

namespace Kaka
{
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoord;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
	};

	struct SpriteVertex
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT2 texCoord;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
	};

	struct BoneVertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoord;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
		unsigned int boneIndices[4] = {};
		float boneWeights[4] = {};
	};
}
