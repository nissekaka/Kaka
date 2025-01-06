#pragma once
#include "Drawable.h"
#include "Core/Graphics/Drawable/ModelData.h"
#include "Core/Graphics/Bindable/BindableCommon.h"
#include <DirectXMath.h>
#include <string>

#include "Graphics/Animation/AnimationPlayer.h"


namespace Kaka
{
	class Graphics;

	class Model : public Drawable
	{
	public:
		Model() = default;
		~Model() override = default;
		void Init();
		void LoadModel(const Graphics& aGfx, const std::string& aFilePath, const bool aAnimated = false);
		bool LoadAnimation(const std::string& aFilePath);
		void SetupModelDrawing(const Graphics& aGfx);
		void Draw(Graphics& aGfx, const float aDeltaTime, bool aFrustumCulling);
		void SetPosition(DirectX::XMFLOAT3 aPosition);
		void SetRotation(DirectX::XMFLOAT3 aRotation);
		void SetScale(float aScale);
		DirectX::XMFLOAT3 GetPosition() const;
		DirectX::XMFLOAT3 GetRotation() const;
		DirectX::XMMATRIX GetTransform() const override;
		float GetScale() const;
		bool IsLoaded() const;
		void BindPixelShader(const Graphics& aGfx);
		//ModelData& GetModelData() { return modelData; }
		AnimatedModelData& GetAnimatedModelData() { return animatedModelData; }
		DirectX::XMMATRIX& GetBoneTransform(int aBoneIndex);
		DirectX::XMMATRIX& GetBoneTransform(const std::string& aBoneName);
		DirectX::XMMATRIX GetBoneWorldTransform(int aBoneIndex) const;
		void DrawDebugAABB(const Graphics& aGfx, const Mesh& aMesh) const;

		static AABB GetTranslatedAABB(const Mesh& aMesh, const DirectX::XMMATRIX& aTransform);

		void SetPixelShader(const Graphics& aGfx, const ePixelShaderType aType);
		void SetTexture(Texture* aTexture);
		void SetTextureAtIndex(Texture* aTexture, size_t aIndex) const;

	public:
		void ShowControlWindow(const char* aWindowName = nullptr);

	private:
		void UpdatePtr(float aDeltaTime);
		void DrawStatic(Graphics& aGfx, bool aFrustumCulling);
		void DrawAnimated(Graphics& aGfx);

	private:
		PixelShader* pixelShader = nullptr;
		VertexShader* vertexShader = nullptr;
		Topology topology = {};

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

		float specularIntensity = 0.1f;
		float specularPower = 2.0f;

	private:
		AnimatedModelData animatedModelData;
		//ModelData modelData;
		eModelType modelType = eModelType::None;
		bool isLoaded = false;

		bool drawSkeleton = false;
		bool drawBoneNames = false;

		std::vector<DirectX::XMMATRIX> combinedTransforms;
		std::vector<DirectX::XMMATRIX> finalTransforms;

		AnimationPlayer animationPlayer;
	};
}
