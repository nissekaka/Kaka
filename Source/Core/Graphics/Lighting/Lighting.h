#pragma once

#include <array>
#include <DirectXMath.h>
#include <string>
#include <Windows.h>

enum class eLightType
{
	Directional,
	Point,
	Spot,
};

static std::string LIGHT_TYPE_NAME[] =
{
	"Directional Light",
	"Point Light",
	"Spot Light"
};

struct DirectionalLightData
{
	DirectX::XMFLOAT3 lightDirection = {}; // 12
	float lightIntensity = {}; // 16
	DirectX::XMFLOAT3 lightColour = {}; // 28
	float ambientLight = {}; // 32
	DirectX::XMMATRIX shadowCamera = {}; // 96
	int numberOfVolumetricSteps = {}; // 100
	float volumetricScattering = {}; // 104
	float volumetricIntensity = {}; // 108
	float padding = {}; // 112
	DirectX::XMFLOAT4 shadowColour; // 128
	DirectX::XMFLOAT4 ambianceColour; // 144
};

struct PointlightData
{
	DirectX::XMFLOAT3 position = {}; // 12
	float intensity = {}; // 16
	DirectX::XMFLOAT3 colour = {}; // 28
	float radius = {}; // 32
	BOOL isActive = false; // 36
	float padding[3] = {}; // 48
};

struct SpotlightData
{
	DirectX::XMFLOAT3 position = {}; // 12
	float intensity = {}; // 16
	DirectX::XMFLOAT3 direction = {}; // 28
	float range = {}; // 32
	DirectX::XMFLOAT3 colour = {}; // 44
	float innerAngle = {}; // 48
	float outerAngle = {}; // 52
	BOOL isActive = false; // 56
	BOOL useTexture = true; // 60
	DirectX::XMMATRIX shadowCamera = {}; // 124
	BOOL useVolumetricLight = false; // 128
	UINT numberOfVolumetricSteps = {}; // 132
	float volumetricScattering = {}; // 136
	float volumetricIntensity = {}; // 140
	float volumetricAngle = {}; // 144
	float volumetricRange = {}; // 148
	float volumetricFade = {}; // 152
	float volumetricAlpha = {}; // 156
	float padding = {}; // 160
};

struct LightConstantBuffer
{
	DirectionalLightData myDirectionalLightData = {};
	std::array<PointlightData, 128u> myPointLightData = {};
	std::array<SpotlightData, 128u> mySpotLightData = {};
	UINT myNumberOfPointLights = {};
	UINT myNumberOfSpotLights = {};
	float padding[2] = {};
};
