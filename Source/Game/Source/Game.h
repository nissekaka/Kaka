#pragma once
#include "Core/Windows/Window.h"
#include "Core/Utility/ImGuiManager.h"
#include "Core/Utility/Timer.h"
#include "Core/Utility/Camera.h"
#include <Core/Utility/KakaMath.h>

#include "Core/Graphics/PostProcessing/PostProcessing.h"
#include "Core/Graphics/Lighting/DirectionalLight.h"
#include "Core/Graphics/Lighting/Pointlight.h"
#include "Core/Graphics/Lighting/Spotlight.h"
#include "Core/Graphics/Lighting/IndirectLighting.h"
#include "Core/Graphics/Drawable/Model.h"
#include "Core/Graphics/Drawable/Terrain.h"
#include "Core/Graphics/Drawable/Skybox.h"
#include "Core/Graphics/Drawable/ReflectionPlane.h"
#include "Core/Graphics/Drawable/SpriteManager.h"
#include "Core/Graphics/Drawable/Sprite.h"
#include "Core/Graphics/Lighting/DeferredLights.h"
#include "Graphics/GraphicsConstants.h"

namespace Kaka
{
	class Game
	{
	public:
		Game();
		// Game loop
		int Go();

	private:
		void Update(const float aDeltaTime);
		void HandleInput(const float aDeltaTime);
		void ShowImGui();
		void ShowStatsWindow();

	private:
		ImGuiManager imGui;
		Window wnd;
		Timer timer;
		Camera camera;

		DeferredLights deferredLights;

		std::vector<Sprite> sprites = {};
		std::vector<Pointlight> pointlights = {};
		std::vector<Spotlight> spotlights = {};

		SpotlightData* flashlightInner = nullptr;
		SpotlightData* flashlightOuter = nullptr;

		float flashlightPositionInterpSpeed = 15.0f;
		float flashlightDirectionInterpSpeed = 5.0f;
		float flashlightBleedAngleMultiplier = 1.9f;
		float flashlightBleedIntensityFactor = 0.05f;
		float flashlightIntensityInterpSpeed = 20.0f;
		float flashlightIntensity;
		bool flashlightOn = true;

		Texture* flashlightTexture;

		Sprite dustParticles;
		Sprite smokeParticles;

	private:
		bool showImGui = true;
		bool showStatsWindow = true;
		bool drawLightDebug = false;
		bool drawRSM = true;

	private:
		DirectX::XMFLOAT3 cameraInput = {0.0f, 0.0f, 0.0f};
		DirectX::XMFLOAT3 cameraVelocity = {0.0f, 0.0f, 0.0f};
		float cameraMoveInterpSpeed = 8.0f;
		float cameraRotateInterpSpeed = 25.0f;
		float cameraSpeed = 0.0f;
		float cameraSpeedDefault = 3.0f;
		float cameraSpeedBoost = 6.0f;

	private:
		std::vector<Model> models;

		Skybox skybox{};
		float skyboxSpeed = 0.005f;
		DirectX::XMFLOAT3 skyboxAngle = {};

		struct CommonBuffer
		{
			DirectX::XMMATRIX viewProjection;
			DirectX::XMMATRIX historyViewProjection;
			DirectX::XMMATRIX inverseViewProjection;
			DirectX::XMMATRIX projection;
			DirectX::XMMATRIX viewInverse;
			DirectX::XMFLOAT4 cameraPosition;
			DirectX::XMFLOAT2 resolution;
			float currentTime;
			float padding;
		};

		CommonBuffer commonBuffer = {};

		PostProcessing postProcessing;
		IndirectLighting indirectLighting;

		struct PostProcessingBuffer
		{
			DirectX::XMFLOAT3 tint; // RGB values for tint adjustment
			float exposure; // Exposure adjustment
			DirectX::XMFLOAT3 blackpoint; // Blackpoint adjustment
			float contrast; // Contrast adjustment
			float saturation; // Saturation adjustment
			float blur; // Blur adjustment
			float sharpness; // Sharpness adjustment
			float padding;
		};

		PostProcessingBuffer ppBuffer = {};

		float cameraMoveSpeed = 10.0f;

		struct ShadowBuffer
		{
			BOOL usePCF = false;
			float offsetScalePCF = 0.004f;
			int sampleCountPCF = 5;
			BOOL usePoisson = true;
			float offsetScalePoissonDisk = 0.0019f;
			float padding[3];
		};

		ShadowBuffer shadowBuffer = {};

		struct RSMBuffer
		{
			BOOL isDirectionalLight = true;
			UINT sampleCount = 12u;
			float rMax = 0.08f; // Maximum sampling radius.
			float rsmIntensity = 10.0f;
			DirectX::XMMATRIX lightCameraTransform;
		};

		RSMBuffer rsmBufferDirectional = {};
		RSMBuffer rsmBufferSpot = {};

		struct RSMLightData
		{
			float colourAndIntensity[4];
			float directionAndInnerAngle[4];
			float lightPositionAndOuterAngle[4];
			float range;
			BOOL isDirectionalLight;
			float padding[2];
		} rsmLightData;

		struct RSMCombinedBuffer
		{
			UINT currentPass = 0;
			float padding[3];
		} rsmCombinedBuffer;

		struct TAABuffer
		{
			DirectX::XMFLOAT2 jitter;
			DirectX::XMFLOAT2 previousJitter;
			BOOL useTAA = true;
			float padding[3];
		} taaBuffer;

		VertexConstantBuffer<CommonBuffer> vcb{wnd.Gfx(), VS_CBUFFER_SLOT_COMMON};
		PixelConstantBuffer<CommonBuffer> pcb{wnd.Gfx(), PS_CBUFFER_SLOT_COMMON};
		PixelConstantBuffer<RSMLightData> rsmLightDataBuffer{wnd.Gfx(), PS_CBUFFER_SLOT_RSM_LIGHT};
		PixelConstantBuffer<TAABuffer> tab{wnd.Gfx(), 1u};
		PixelConstantBuffer<RSMBuffer> rsmPixelBuffer{wnd.Gfx(), PS_CBUFFER_SLOT_RSM_DIRECTIONAL};
		PixelConstantBuffer<ShadowBuffer> shadowPixelBuffer{wnd.Gfx(), PS_CBUFFER_SLOT_SHADOW};
		PixelConstantBuffer<PostProcessingBuffer> ppb{wnd.Gfx(), 1u};

		static constexpr unsigned int SAMPLE_COUNT_DIRECTIONAL = 8u;
		static constexpr unsigned int SAMPLE_COUNT_SPOT = 4u;


		bool flipFlop = false;
	};
}
