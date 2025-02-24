#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>

namespace Kaka
{
	struct Keyframe
	{
		float time = 0.0f;
		std::vector<DirectX::XMMATRIX> boneTransforms{};
	};

	struct AnimationClip
	{
		std::string name = "None";
		int frames = 0;
		float fps = 0.0f;
		float duration = 0.0f;
		std::vector<Keyframe> keyframes{};
	};
}
