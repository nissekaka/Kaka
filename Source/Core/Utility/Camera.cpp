#include "stdafx.h"
#include "Camera.h"
#include "Core/Utility/KakaMath.h"
#include "External/include/imgui/imgui.h"
#include <algorithm>

namespace Kaka
{
	Camera::Camera()
	{
		Reset();
	}

	void Camera::ShowControlWindow()
	{
		if (ImGui::Begin("Camera"))
		{
			ImGui::Text("Position");
			ImGui::DragFloat3("XYZ", &position.x);
			ImGui::Text("Orientation");
			ImGui::SliderAngle("Pitch", &pitch, 0.995f * -90.0f, 0.995f * 90.0f);
			ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
			if (ImGui::Button("Reset"))
			{
				Reset();
			}
		}
		ImGui::End();
	}

	void Camera::Reset()
	{
		position = {0.0f, 0.0f, -4.0f};
		//position = {0.0f,140.0f,-450.0f};
		pitch = 0.0f;
		yaw = -PI * 2 / 5;
	}

	void Camera::Rotate(const float aDx, const float aDy)
	{
		yaw = WrapAngle(yaw + aDx * ROTATION_SPEED);
		pitch = std::clamp(pitch + aDy * ROTATION_SPEED, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
	}

	void Camera::Translate(DirectX::XMFLOAT3 aTranslation)
	{
		DirectX::XMStoreFloat3(&aTranslation, DirectX::XMVector3Transform(
			                       DirectX::XMLoadFloat3(&aTranslation),
			                       DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) *
			                       DirectX::XMMatrixScaling(TRAVEL_SPEED, TRAVEL_SPEED, TRAVEL_SPEED)
		                       ));

		position = {
			position.x + aTranslation.x,
			position.y + aTranslation.y,
			position.z + aTranslation.z
		};
	}

	void Camera::SetPosition(const DirectX::XMFLOAT3 aPosition)
	{
		position = aPosition;
	}

	void Camera::SetRotationDegrees(const float aPitch, const float aYaw)
	{
		pitch = DegToRad(aPitch);
		yaw = DegToRad(aYaw);
	}

	void Camera::SetDirection(const DirectX::XMFLOAT3 aDirection)
	{
		DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&aDirection);
		direction = DirectX::XMVector3Normalize(direction);

		// Needed to normalize this for directional light shadow camera
		// pitch would be -nan(ind) without this
		pitch = asin(-DirectX::XMVectorGetY(direction));
		// Not sure if needed for yaw but it's here for now
		yaw = atan2(-DirectX::XMVectorGetX(direction), DirectX::XMVectorGetZ(direction));
		//yaw = atan2(aDirection.x, aDirection.z);
	}

	DirectX::XMMATRIX Camera::GetView() const
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();

		matrix *= DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
		matrix *= DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f);
		matrix *= DirectX::XMMatrixTranslation(position.x, position.y, position.z);

		return matrix;
	}

	DirectX::XMMATRIX Camera::GetInverseView() const
	{
		return DirectX::XMMatrixInverse(nullptr, GetView());
	}

	DirectX::XMMATRIX Camera::GetProjection() const
	{
		return projection;
	}

	DirectX::XMMATRIX Camera::GetJitteredProjection() const
	{
		return jitteredProjection;
	}

	DirectX::XMFLOAT3 Camera::GetPosition() const
	{
		return position;
	}

	// R  U  F
	// 00 01 02 03
	// 10 11 12 13
	// 20 21 22 23
	// 30 31 32 33

	DirectX::XMVECTOR Camera::GetForwardVector() const
	{
		return {
			GetInverseView().r[0].m128_f32[2],
			GetInverseView().r[1].m128_f32[2],
			GetInverseView().r[2].m128_f32[2]
		};
	}

	DirectX::XMVECTOR Camera::GetRightVector() const
	{
		return {
			GetInverseView().r[0].m128_f32[0],
			GetInverseView().r[1].m128_f32[0],
			GetInverseView().r[2].m128_f32[0]
		};
	}

	DirectX::XMVECTOR Camera::GetUpVector() const
	{
		return {
			GetInverseView().r[0].m128_f32[1],
			GetInverseView().r[1].m128_f32[1],
			GetInverseView().r[2].m128_f32[1]
		};
	}

	void Camera::SetPerspective(const float aWidth, const float aHeight, const float aVFov, const float aNearZ, const float aFarZ)
	{
		cameraType = eCameraType::Perspective;

		perspectiveData.width = aWidth;
		perspectiveData.height = aHeight;
		perspectiveData.nearZ = aNearZ;
		perspectiveData.farZ = aFarZ;

		// Convert aVFov to radians if it's specified in degrees
		const float vFovRadians = DirectX::XMConvertToRadians(aVFov);

		const float verticalFov = 2 * atan(tan(vFovRadians / 2) * (aHeight / aWidth));

		perspectiveData.vFov = verticalFov;

		projection = DirectX::XMMatrixPerspectiveFovLH(verticalFov, aWidth / aHeight, aNearZ, aFarZ);
		jitteredProjection = projection;
	}

	void Camera::SetOrthographic(const float aWidth, const float aHeight, const float aNearZ, const float aFarZ)
	{
		cameraType = eCameraType::Orthographic;

		orthographicData.width = aWidth;
		orthographicData.height = aHeight;
		orthographicData.nearZ = aNearZ;
		orthographicData.farZ = aFarZ;

		projection = DirectX::XMMatrixOrthographicLH(aWidth, aHeight, aNearZ, aFarZ);
		jitteredProjection = projection;
	}

	void Camera::ApplyProjectionJitter(const float aJitterX, const float aJitterY)
	{
		const DirectX::XMMATRIX offset = DirectX::XMMatrixTranslation(aJitterX, aJitterY, 0.0f);

		jitteredProjection = projection * offset;
	}
}
