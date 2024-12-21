#include "stdafx.h"
#include "AnimationPlayer.h"

#include "Core/Graphics/Drawable/ModelData.h"

namespace Kaka
{
	void AnimationPlayer::Init(AnimatedModelDataPtr* aModelData)
	{
		modelData = aModelData;
		currentAnimation.combinedTransforms.resize(modelData->skeleton->bones.size());
		currentAnimation.finalTransforms.resize(modelData->skeleton->bones.size());
		blendAnimation.combinedTransforms.resize(modelData->skeleton->bones.size());
		blendAnimation.finalTransforms.resize(modelData->skeleton->bones.size());

		if (useBlendTree)
		{
			StartAnimation(modelData->animationNames[0], currentAnimation);
			StartAnimation(modelData->animationNames[1], blendAnimation);
		}
	}

	void AnimationPlayer::Animate(const float aDeltaTime)
	{
		if (modelData == nullptr)
		{
			return;
		}

		if (modelData->skeleton == nullptr)
		{
			return;
		}

		const Skeleton* skeleton = modelData->skeleton;

		if (UpdateAnimation(aDeltaTime, currentAnimation))
		{
			isAnimationPlaying = true;

			if (useBlendTree)
			{
				if (UpdateAnimation(aDeltaTime, blendAnimation))
				{
					BlendPoses(currentAnimation, blendAnimation, blendFactor);
				}
			}
			else
			{
				modelData->combinedTransforms = currentAnimation.combinedTransforms;
				modelData->finalTransforms = currentAnimation.finalTransforms;

				if (isBlending)
				{
					if (UpdateAnimation(aDeltaTime, blendAnimation))
					{
						blendTimer += aDeltaTime;
						const float blendFactor = (std::min)(1.0f, blendTimer / blendTime);

						BlendPoses(currentAnimation, blendAnimation, blendFactor);

						// If blending is complete, reset variables
						if (blendFactor >= 1.0f)
						{
							isBlending = false;
							blendTimer = 0.0f;
							currentAnimation.clip = blendAnimation.clip;
							currentAnimation.time = blendAnimation.time;
							currentAnimation.speed = blendAnimation.speed;
							currentAnimation.isPlaying = blendAnimation.isPlaying;
							currentAnimation.isLooping = blendAnimation.isLooping;
							blendAnimation.clip = nullptr;
						}
					}
				}
			}
		}
		else
		{
			// Show bind pose (T-pose)
			isAnimationPlaying = false;
			for (size_t i = 0; i < skeleton->bones.size(); i++)
			{
				// This works because the vertex is not moved in the vertex shader
				modelData->combinedTransforms[i] = DirectX::XMMatrixInverse(nullptr, modelData->skeleton->bones[i].bindPose);
				modelData->finalTransforms[i] = DirectX::XMMatrixIdentity();
			}
		}
	}

	bool AnimationPlayer::UpdateAnimation(const float aDeltaTime, Animation& aOutAnimation) const
	{
		const Skeleton* skeleton = modelData->skeleton;

		if (aOutAnimation.clip != nullptr)
		{
			// Apply animation transformation based on the current time
			if (aOutAnimation.isPlaying)
			{
				aOutAnimation.time += aDeltaTime * aOutAnimation.speed;

				if (aOutAnimation.isLooping)
				{
					if (aOutAnimation.time >= aOutAnimation.clip->duration)
					{
						aOutAnimation.time = 0.0f;
					}
				}
				else
				{
					if (aOutAnimation.time >= aOutAnimation.clip->duration)
					{
						aOutAnimation.time = aOutAnimation.clip->duration;
						return true;
					}
				}

				// Calculate the current frame and delta
				const float frameRate = 1.0f / aOutAnimation.clip->fps;
				const float result = aOutAnimation.time / frameRate;
				const size_t frame = (std::size_t)std::floor(result); // Current frame
				const float delta = result - static_cast<float>(frame); // Progress to the next frame

				// Interpolate between current and next frame
				for (size_t i = 0; i < skeleton->bones.size(); i++)
				{
					DirectX::XMMATRIX currentFramePose = aOutAnimation.clip->keyframes[frame].boneTransforms[i];
					DirectX::XMMATRIX nextFramePose = aOutAnimation.clip->keyframes[(frame + 1) % aOutAnimation.clip->keyframes.size()].boneTransforms[i];

					// Interpolate between current and next frame using delta
					DirectX::XMMATRIX blendedPose = currentFramePose + delta * (nextFramePose - currentFramePose);

					const int parentIndex = skeleton->bones[i].parentIndex;

					if (parentIndex >= 0)
					{
						// Accumulate relative transformation
						aOutAnimation.combinedTransforms[i] = blendedPose * aOutAnimation.combinedTransforms[parentIndex];
					}
					else
					{
						// Root bone, use absolute transformation
						aOutAnimation.combinedTransforms[i] = blendedPose;
					}

					aOutAnimation.finalTransforms[i] = skeleton->bones[i].bindPose * aOutAnimation.combinedTransforms[i];
				}
			}
			else
			{
				// Play paused frame i.e do nothing
			}
			return true;
		}
		aOutAnimation.time = 0.0f;

		return false;
	}

	void AnimationPlayer::BlendPoses(Animation& aFromAnimation, Animation& aToAnimation, const float aBlendFactor) const
	{
		const Skeleton* skeleton = modelData->skeleton;

		for (size_t i = 0; i < skeleton->bones.size(); i++)
		{
			{
				const DirectX::XMMATRIX currentFramePose = aFromAnimation.combinedTransforms[i];
				const DirectX::XMMATRIX blendFramePose = aToAnimation.combinedTransforms[i];

				// Blended pose needs to be multiplication of decomposed matrices
				DirectX::XMVECTOR currentScale, currentRotation, currentTranslation;
				DirectX::XMVECTOR blendScale, blendRotation, blendTranslation;
				DirectX::XMMatrixDecompose(&currentScale, &currentRotation, &currentTranslation, currentFramePose);
				DirectX::XMMatrixDecompose(&blendScale, &blendRotation, &blendTranslation, blendFramePose);

				const DirectX::XMMATRIX blendedPose = DirectX::XMMatrixAffineTransformation(
					DirectX::XMVectorLerp(currentScale, blendScale, aBlendFactor),
					DirectX::XMVectorZero(),
					DirectX::XMQuaternionSlerp(currentRotation, blendRotation, aBlendFactor),
					DirectX::XMVectorLerp(currentTranslation, blendTranslation, aBlendFactor)
				);

				modelData->finalTransforms[i] = skeleton->bones[i].bindPose * blendedPose;
			}
		}
	}

	void AnimationPlayer::PlayAnimation(const std::string& aAnimationName, const bool aShouldLoop, const float aSpeed)
	{
		if (isBlending)
		{
			isBlending = false;
			currentAnimation.clip = modelData->animationClipMap[aAnimationName];
			currentAnimation.time = 0.0f;
			currentAnimation.speed = aSpeed;
			currentAnimation.isPlaying = true;
			currentAnimation.isLooping = aShouldLoop;
			return;
		}

		if (modelData->animationClipMap.empty())
		{
			return;
		}

		if (currentAnimation.clip != nullptr && currentAnimation.clip->name == aAnimationName)
		{
			return;
		}

		if (currentAnimation.clip != nullptr)
		{
			PlayAnimationBlend(aAnimationName, aShouldLoop, aSpeed, blendTime);
			return;
		}

		currentAnimation.clip = modelData->animationClipMap[aAnimationName];
		currentAnimation.time = 0.0f;
		currentAnimation.speed = aSpeed;
		currentAnimation.isPlaying = true;
		currentAnimation.isLooping = aShouldLoop;
	}

	bool AnimationPlayer::StartAnimation(const std::string& aAnimationName, Animation& aAnimation, bool aShouldLoop, float aSpeed)
	{
		if (modelData->animationClipMap.empty())
		{
			return false;
		}

		if (aAnimation.clip != nullptr && aAnimation.clip->name == aAnimationName)
		{
			return false;
		}

		aAnimation.clip = modelData->animationClipMap[aAnimationName];
		aAnimation.time = 0.0f;
		aAnimation.speed = aSpeed;
		aAnimation.isPlaying = true;
		aAnimation.isLooping = aShouldLoop;

		return true;
	}

	void AnimationPlayer::PlayAnimationBlend(const std::string& aAnimationName, const bool aShouldLoop,
	                                         const float aSpeed, const float aBlendTime)
	{
		if (modelData->animationClipMap.empty())
		{
			return;
		}

		if (currentAnimation.clip == nullptr)
		{
			PlayAnimation(aAnimationName, aShouldLoop, aSpeed);
			return;
		}

		blendAnimation.clip = modelData->animationClipMap[aAnimationName];
		isBlending = true;
		blendTime = aBlendTime;
		blendAnimation.time = 0.0f;
		blendAnimation.speed = aSpeed;
		blendAnimation.isPlaying = true;
		blendAnimation.isLooping = aShouldLoop;
	}

	void AnimationPlayer::PauseAnimation()
	{
		currentAnimation.isPlaying = false;
	}

	void AnimationPlayer::ResumeAnimation()
	{
		currentAnimation.isPlaying = true;
	}

	void AnimationPlayer::StopAnimation()
	{
		currentAnimation.clip = nullptr;
	}

	void AnimationPlayer::SetAnimationShouldLoop(const bool aShouldLoop)
	{
		currentAnimation.isLooping = aShouldLoop;
	}

	void AnimationPlayer::SetAnimationSpeed(const float aSpeed)
	{
		currentAnimation.speed = aSpeed;
	}

	void AnimationPlayer::SetBlendTime(const float aBlendTime)
	{
		blendTime = aBlendTime;
	}
}
