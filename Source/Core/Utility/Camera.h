#pragma once
#include "Core/Graphics/Graphics.h"
#include "Graphics/Drawable/ModelData.h"


namespace Kaka
{
	struct Mesh;

	struct PerspectiveData
	{
		float width;
		float height;
		float vFov;
		float nearZ;
		float farZ;
	};

	struct OrthographicData
	{
		float width;
		float height;
		float nearZ;
		float farZ;
	};

	enum class eCameraType
	{
		Perspective,
		Orthographic
	};

	class Camera
	{
		struct FrustumPlanes
		{
			DirectX::XMFLOAT4 planes[6];
		};

	public:
		Camera();
		void ShowControlWindow();
		void Reset();
		void Rotate(float aDx, float aDy);
		void Translate(DirectX::XMFLOAT3 aTranslation);
		void SetPosition(DirectX::XMFLOAT3 aPosition);
		void SetRotationDegrees(float aPitch, float aYaw);
		void SetDirection(DirectX::XMFLOAT3 aDirection);
		DirectX::XMMATRIX GetView() const;
		DirectX::XMMATRIX GetInverseView() const;
		DirectX::XMMATRIX GetProjection() const;
		DirectX::XMMATRIX GetJitteredProjection() const;
		DirectX::XMFLOAT3 GetPosition() const;
		DirectX::XMVECTOR GetForwardVector() const;
		DirectX::XMVECTOR GetRightVector() const;
		DirectX::XMVECTOR GetUpVector() const;
		void SetPerspective(float aWidth, float aHeight, float aVFov, float aNearZ, float aFarZ);
		void SetOrthographic(float aWidth, float aHeight, float aNearZ, float aFarZ);
		void ApplyProjectionJitter(float aJitterX, float aJitterY);
		//void ResetFrustumFlag();
		void MarkDirty() { dirty = true; }

	private:
		PerspectiveData perspectiveData;
		OrthographicData orthographicData;
		eCameraType cameraType;
		DirectX::XMMATRIX projection{};
		DirectX::XMMATRIX jitteredProjection{};
		DirectX::XMMATRIX unjitteredProjection{};

		DirectX::XMFLOAT3 position;
		float pitch;
		float yaw;
		static constexpr float TRAVEL_SPEED = 12.0f;
		static constexpr float ROTATION_SPEED = 0.0008f;

		FrustumPlanes ExtractFrustumPlanes();

	public:
		bool IsPointInFrustum(const DirectX::XMFLOAT3& aPoint);
		bool IsBoundingBoxInFrustum(const DirectX::XMFLOAT3& aMin, const DirectX::XMFLOAT3& aMax);
		bool IsBoundingBoxInFrustum(const AABB& aAabb);
		std::vector<bool> AreMeshesInFrustum(const std::vector<Mesh>& aMeshes, const DirectX::XMMATRIX& aObjectToWorld);

	public:
		DirectX::XMFLOAT3 cameraInput = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 cameraVelocity = { 0.0f, 0.0f, 0.0f };
		float cameraMoveInterpSpeed = 8.0f;
		float cameraRotateInterpSpeed = 25.0f;
		float cameraSpeed = 0.0f;
		float cameraSpeedDefault = 3.0f;
		float cameraSpeedBoost = 6.0f;

		FrustumPlanes frustum;
		//bool extractedFrustumThisFrame = false;
		bool dirty = true;
	};
}
