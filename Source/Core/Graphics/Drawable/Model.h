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
		enum class eShaderType { Solid, Light, Phong, AnimPhong, PBR, AnimPBR };

	public:
		Model() = default;
		Model(const Graphics& aGfx, const std::string& aFilePath, const eShaderType aShaderType);
		~Model() override = default;
		void Init();
		void LoadModel(const Graphics& aGfx, const std::string& aFilePath, const eShaderType aShaderType);
		bool LoadAnimation(const std::string& aFilePath);
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
		ModelDataPtr& GetModelData() { return modelData; }
		AnimatedModelDataPtr& GetAnimatedModelData() { return animatedModelData; }
		DirectX::XMMATRIX& GetBoneTransform(int aBoneIndex);
		DirectX::XMMATRIX& GetBoneTransform(const std::string& aBoneName);
		DirectX::XMMATRIX GetBoneWorldTransform(int aBoneIndex) const;
		void DrawDebugAABB(const Graphics& aGfx, const Mesh& aMesh) const;

		AABB GetTranslatedAABB(const Mesh& aMesh) const;

		void SetPixelShader(const Graphics& aGfx, const std::wstring& aFilePath);
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
		std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
		InputLayout inputLayout;
		Topology topology = {};

	private:
		eShaderType shaderType = {};

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
		AnimatedModelDataPtr animatedModelData;
		ModelDataPtr modelData;
		eModelType modelType = eModelType::None;
		bool isLoaded = false;

		bool drawSkeleton = false;
		bool drawBoneNames = false;

		std::vector<DirectX::XMMATRIX> combinedTransforms;
		std::vector<DirectX::XMMATRIX> finalTransforms;

		AnimationPlayer animationPlayer;
	};
}
