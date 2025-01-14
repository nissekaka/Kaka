#include "stdafx.h"
#include "Camera.h"
#include "Core/Utility/KakaMath.h"
#include "External/include/imgui/imgui.h"
#include <algorithm>

#include "Graphics/Drawable/Model.h"

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

		MarkDirty();
	}

	void Camera::Rotate(const float aDx, const float aDy)
	{
		yaw = WrapAngle(yaw + aDx * ROTATION_SPEED);
		pitch = std::clamp(pitch + aDy * ROTATION_SPEED, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);

		MarkDirty();
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

		MarkDirty();

	}

	void Camera::SetPosition(const DirectX::XMFLOAT3 aPosition)
	{
		position = aPosition;

		MarkDirty();
	}

	void Camera::SetRotationDegrees(const float aPitch, const float aYaw)
	{
		pitch = DegToRad(aPitch);
		yaw = DegToRad(aYaw);

		MarkDirty();
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

		MarkDirty();
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

	Camera::FrustumPlanes Camera::ExtractFrustumPlanes() 
	{
		if (!dirty)
		{
			return frustum;
		}

		//extractedFrustumThisFrame = true;
		dirty = false;

		// Extract the rows of the view-projection matrix
		DirectX::XMFLOAT4X4 VP;
		const DirectX::XMMATRIX viewProjectionMatrix = GetInverseView() * GetProjection();
		DirectX::XMStoreFloat4x4(&VP, viewProjectionMatrix);

		// Extract the frustum planes from the view-projection matrix
		frustum.planes[0] = DirectX::XMFLOAT4(VP._14 + VP._11, VP._24 + VP._21, VP._34 + VP._31, VP._44 + VP._41);
		// Left plane
		frustum.planes[1] = DirectX::XMFLOAT4(VP._14 - VP._11, VP._24 - VP._21, VP._34 - VP._31, VP._44 - VP._41);
		// Right plane
		frustum.planes[2] = DirectX::XMFLOAT4(VP._14 - VP._12, VP._24 - VP._22, VP._34 - VP._32, VP._44 - VP._42);
		// Top plane
		frustum.planes[3] = DirectX::XMFLOAT4(VP._14 + VP._12, VP._24 + VP._22, VP._34 + VP._32, VP._44 + VP._42);
		// Bottom plane
		frustum.planes[4] = DirectX::XMFLOAT4(VP._13, VP._23, VP._33, VP._43); // Near plane
		frustum.planes[5] = DirectX::XMFLOAT4(VP._14 - VP._13, VP._24 - VP._23, VP._34 - VP._33, VP._44 - VP._43);
		// Far plane

		// Normalize the frustum planes
		for (DirectX::XMFLOAT4& plane : frustum.planes)
		{
			const float length = std::sqrt(
				plane.x * plane.x +
				plane.y * plane.y +
				plane.z * plane.z);

			plane = DirectX::XMFLOAT4(plane.x / length,
				plane.y / length,
				plane.z / length,
				plane.w / length);
		}

		return frustum;
	}

	bool Camera::IsPointInFrustum(const DirectX::XMFLOAT3& aPoint)
	{
		const FrustumPlanes frustum = ExtractFrustumPlanes();
		for (const DirectX::XMFLOAT4& plane : frustum.planes)
		{
			if (plane.x * aPoint.x + plane.y * aPoint.y + plane.z * aPoint.z + plane.w > 0.0f)
				continue;

			// If the point is outside any frustum plane, it is not visible
			return false;
		}

		// If the point is not outside any frustum plane, it is visible
		return true;
	}

	bool Camera::IsBoundingBoxInFrustum(const DirectX::XMFLOAT3& aMin, const DirectX::XMFLOAT3& aMax)
	{
		const FrustumPlanes frustum = ExtractFrustumPlanes();
		for (const DirectX::XMFLOAT4& plane : frustum.planes)
		{
			if (plane.x * aMin.x + plane.y * aMin.y + plane.z * aMin.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMax.x + plane.y * aMin.y + plane.z * aMin.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMin.x + plane.y * aMax.y + plane.z * aMin.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMax.x + plane.y * aMax.y + plane.z * aMin.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMin.x + plane.y * aMin.y + plane.z * aMax.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMax.x + plane.y * aMin.y + plane.z * aMax.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMin.x + plane.y * aMax.y + plane.z * aMax.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMax.x + plane.y * aMax.y + plane.z * aMax.z + plane.w > 0.0f)
				continue;

			// If the bounding box is completely outside any frustum plane, it is not visible
			return false;
		}

		// If the bounding box is not completely outside any frustum plane, it is visible
		return true;
	}

	bool Camera::IsBoundingBoxInFrustum(const AABB& aAabb)
	{
		const DirectX::XMFLOAT3 aMin = aAabb.minBound;
		const DirectX::XMFLOAT3 aMax = aAabb.maxBound;

		const FrustumPlanes frustum = ExtractFrustumPlanes();
		for (const DirectX::XMFLOAT4& plane : frustum.planes)
		{
			if (plane.x * aMin.x + plane.y * aMin.y + plane.z * aMin.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMax.x + plane.y * aMin.y + plane.z * aMin.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMin.x + plane.y * aMax.y + plane.z * aMin.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMax.x + plane.y * aMax.y + plane.z * aMin.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMin.x + plane.y * aMin.y + plane.z * aMax.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMax.x + plane.y * aMin.y + plane.z * aMax.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMin.x + plane.y * aMax.y + plane.z * aMax.z + plane.w > 0.0f)
				continue;
			if (plane.x * aMax.x + plane.y * aMax.y + plane.z * aMax.z + plane.w > 0.0f)
				continue;

			// If the bounding box is completely outside any frustum plane, it is not visible
			return false;
		}

		// If the bounding box is not completely outside any frustum plane, it is visible
		return true;
	}

	std::vector<bool> Camera::AreMeshesInFrustum(const std::vector<Mesh>& aMeshes, const DirectX::XMMATRIX& aObjectToWorld)
	{
		const FrustumPlanes frustum = ExtractFrustumPlanes();

		std::vector<bool> results;
		results.resize(aMeshes.size());

		//for (const Mesh& mesh : aMeshes)
		for (int i = 0; i < aMeshes.size(); ++i)
		{
			bool isVisible = true;

			const DirectX::XMFLOAT3 minBound = Model::GetTranslatedAABB(aMeshes[i], aObjectToWorld).minBound;
			const DirectX::XMFLOAT3 maxBound = Model::GetTranslatedAABB(aMeshes[i], aObjectToWorld).maxBound;

			for (const DirectX::XMFLOAT4& plane : frustum.planes)
			{

				if (plane.x * minBound.x + plane.y * minBound.y + plane.z * minBound.z + plane.w > 0.0f)
					continue;
				if (plane.x * maxBound.x + plane.y * minBound.y + plane.z * minBound.z + plane.w > 0.0f)
					continue;
				if (plane.x * minBound.x + plane.y * maxBound.y + plane.z * minBound.z + plane.w > 0.0f)
					continue;
				if (plane.x * maxBound.x + plane.y * maxBound.y + plane.z * minBound.z + plane.w > 0.0f)
					continue;
				if (plane.x * minBound.x + plane.y * minBound.y + plane.z * maxBound.z + plane.w > 0.0f)
					continue;
				if (plane.x * maxBound.x + plane.y * minBound.y + plane.z * maxBound.z + plane.w > 0.0f)
					continue;
				if (plane.x * minBound.x + plane.y * maxBound.y + plane.z * maxBound.z + plane.w > 0.0f)
					continue;
				if (plane.x * maxBound.x + plane.y * maxBound.y + plane.z * maxBound.z + plane.w > 0.0f)
					continue;

				// If the bounding box is completely outside any frustum plane, it is not visible
				isVisible = false;
				break;
			}

			results[i] = isVisible;
		}

		return results;
	}
}
