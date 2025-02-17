#pragma once

namespace Kaka::Ecs
{
	struct TransformComponent
	{
	private:
		DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4 rotation{ 0.0f, 0.0f, 0.0f, 1.0f }; // Quaternion
		DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };

	public:
		static DirectX::XMFLOAT4 EulerToQuaternion(const DirectX::XMFLOAT3& aEulerAngles)
		{
			using namespace DirectX;
			const XMVECTOR quaternion = XMQuaternionRotationRollPitchYaw(aEulerAngles.x, aEulerAngles.y, aEulerAngles.z);
			XMFLOAT4 result;
			XMStoreFloat4(&result, quaternion);
			return result;
		}

		static DirectX::XMFLOAT3 QuaternionToEuler(const DirectX::XMFLOAT4& aQuaternion)
		{
			using namespace DirectX;
			const XMVECTOR q = XMLoadFloat4(&aQuaternion);

			XMFLOAT3 angles;
			const XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(q);
			XMStoreFloat3(&angles, XMQuaternionRotationMatrix(rotationMatrix));
			return angles;
		}

	public:
		const DirectX::XMFLOAT3& GetPosition() const { return position; }
		const DirectX::XMFLOAT4& GetQuaternionRotation() const { return rotation; }
		DirectX::XMFLOAT3 GetEulerRotation() const { return QuaternionToEuler(rotation); }
		const DirectX::XMFLOAT3& GetScale() const { return scale; }

		void SetPosition(const DirectX::XMFLOAT3& aNewPosition) { position = aNewPosition; }
		void SetPosition(const float aX, const float aY, const float aZ) { position = { aX, aY, aZ }; }
		void SetPositionX(const float aX) { position.x = aX; }
		void SetPositionY(const float aY) { position.y = aY; }
		void SetPositionZ(const float aZ) { position.z = aZ; }

		void SetQuaternionRotation(const DirectX::XMFLOAT4& aNewRotation) { rotation = aNewRotation; }
		void SetEulerRotation(const DirectX::XMFLOAT3& aNewRotation) { rotation = EulerToQuaternion(aNewRotation); }

		void SetScale(const DirectX::XMFLOAT3& aNewScale) { scale = aNewScale; }
		void SetScale(const float aScale) { scale = { aScale, aScale, aScale }; }
		void SetScale(const float aX, const float aY, const float aZ) { scale = { aX, aY, aZ }; }

		DirectX::XMMATRIX GetObjectToWorldMatrix() const
		{
			const DirectX::XMVECTOR vposition = DirectX::XMLoadFloat3(&position);
			const DirectX::XMVECTOR vrotation = DirectX::XMLoadFloat4(&rotation);
			const DirectX::XMVECTOR vscale = DirectX::XMLoadFloat3(&scale);

			const DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(vposition);
			const DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(vrotation);
			const DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScalingFromVector(vscale);

			return scaleMatrix * rotationMatrix * translationMatrix;
		}
	};
}