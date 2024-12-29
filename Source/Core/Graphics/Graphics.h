#pragma once
#include <wrl.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <memory>
#include <vector>
#include <External/Include/FileWatch/FileWatch.hpp>

#include "Core/Graphics/GraphicsConstants.h"
#include "Core/Graphics/GBuffer.h"
#include "Core/Graphics/RSMBuffer.h"
#include "Core/Graphics/ShadowBuffer.h"
#include "Core/Graphics/ShaderFactory.h"
#include "Core/Graphics/PostProcessing/PostProcessing.h"
#include "Core/Graphics/Lighting/DeferredLights.h"
#include "Core/Graphics/Lighting/IndirectLighting.h"
#include "Core/Graphics/Bindable/ConstantBuffers.h"
#include "Core/Graphics/Bindable/DepthStencil.h"
#include "Core/Graphics/Bindable/BlendState.h"
#include "Core/Graphics/Drawable/Skybox.h"
#include "Core/Graphics/Drawable/Model.h"

#define KAKA_BG_COLOUR {0.1f, 0.2f, 0.3f, 1.0f}

namespace DirectX
{
	struct XMFLOAT3;
}

namespace Kaka
{
	class Texture;
	class VertexShader;
	class PixelShader;
	class Camera;
	struct Mesh;

	enum class eRenderTargetType
	{
		None,
		Default,
		PostProcessing,
		HistoryN1,
		HistoryN
	};

	struct RenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource;
	};

	class Graphics
	{
		friend class GBuffer;
		friend class RSMBuffer;
		friend class ShadowBuffer;
		friend class IndirectLighting;
		friend class DeferredLights;
		friend class Model;
		friend class Bindable;
		friend class DirectionalLight;
		friend class Pointlight;
		friend class Spotlight;
		friend class Terrain;
		friend class Skybox;
		friend class ReflectionPlane;
		friend class Game;
		friend class PostProcessing;
		friend class Sprite;

	public:
		Graphics(HWND aHWnd, UINT aWidth, UINT aHeight);
		Graphics(const Graphics&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		~Graphics();
		void BeginFrame();
		void EndFrame();
		void DrawIndexed(UINT aCount);
		void DrawIndexedInstanced(UINT aCount, UINT aInstanceCount);
		DirectX::XMMATRIX GetProjection() const;
		DirectX::XMMATRIX GetJitteredProjection() const;
		void SetupCamera(const float aWidth, const float aHeight, const float aFoV = 80.0f, const float aNearZ = 0.5f, const float aFarZ = 5000.0f);
		void SetCamera(Camera& aCamera);
		//Camera& GetCamera() { return &currentCamera; }
		DirectX::XMMATRIX GetCameraInverseView() const;
		UINT GetDrawcallCount() const;
		void SetRenderTarget(eRenderTargetType aRenderTargetType, ID3D11DepthStencilView* aDepth) const;
		void SetRenderTargetShadow(const RSMBuffer& aBuffer) const;
		void ApplyProjectionJitter();

		void HandleBloomScaling(PostProcessing& aPostProcessor, ID3D11ShaderResourceView* aResource);

		void SetBlendState(eBlendStates aBlendState);
		void SetDepthStencilState(eDepthStencilStates aDepthStencilState);
		void SetRasterizerState(eRasterizerStates aRasterizerState);

		void BindPostProcessingTexture();
		void BindBloomDownscaleTexture(const int aIndex);
		DirectX::XMFLOAT2 GetCurrentResolution() const;

		void StartShadows(Camera& aCamera, const DirectX::XMFLOAT3 aLightDirection, const ShadowBuffer& aBuffer, UINT aSlot);
		void ResetShadows(Camera& aCamera);
		void BindShadows(const ShadowBuffer& aBuffer, UINT aSlot);
		void UnbindShadows(UINT aSlot);

		void SetPixelShaderOverride(const std::wstring& aFileName) { pixelShaderOverride = ShaderFactory::GetPixelShader(*this, aFileName); }
		void SetVertexShaderOverride(const std::wstring& aFileName) { vertexShaderOverride = ShaderFactory::GetVertexShader(*this, aFileName); }
		PixelShader* GetPixelShaderOverride() const { return pixelShaderOverride; }
		VertexShader* GetVertexShaderOverride() const { return vertexShaderOverride; }
		bool HasPixelShaderOverride() const { return pixelShaderOverride != nullptr; }
		bool HasVertexShaderOverride() const { return vertexShaderOverride != nullptr; }
		void ClearPixelShaderOverride() { pixelShaderOverride = nullptr; }
		void ClearVertexShaderOverride() { vertexShaderOverride = nullptr; }

		void ShowImGui(const float aFPS);
		void ShowStatsWindow(const float aFPS);

	private:
		bool showImGui = true;
		bool showStatsWindow = true;
		bool drawLightDebug = false;
	public:
		void UpdateLights(const float aDeltaTime);
		void Render(const float aDeltaTime, const float aTotalTime, const float aFPS);
		void EnableImGui();
		void DisableImGui();
		bool IsImGuiEnabled() const;
		UINT GetWidth() const;
		UINT GetHeight() const;
		void ProcessFileChangeEngine(const std::wstring& aPath, filewatch::Event aEvent);

	private:
		bool imGuiEnabled = true;
		UINT drawcallCount;

	private:
		struct FrustumPlanes
		{
			DirectX::XMFLOAT4 planes[6];
		};

		FrustumPlanes ExtractFrustumPlanes() const;

	public:
		bool IsBoundingBoxInFrustum(const DirectX::XMFLOAT3& aMin, const DirectX::XMFLOAT3& aMax) const;

	private:
		filewatch::FileWatch<std::wstring> shaderFileWatcher;

		Camera camera;
		Camera* currentCamera = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;

		DeferredLights deferredLights;

		RenderTarget historyNTarget;
		RenderTarget historyN1Target;

		DirectX::XMMATRIX historyViewProjection;

		std::vector<RenderTarget> bloomDownscaleTargets = {};

		struct DownSampleData
		{
			float bloomBlending = 0.0f;
			float bloomThreshold = 0.1f;
			int uvScale = 2;
			float padding;
		} downSampleData;

		int bloomDivideFactor = 2;
		bool usePostProcessing = true;
		int bloomSteps = 5;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pDefaultTarget;

		Sampler defaultSampler;
		Sampler linearSampler;
		Sampler pointClampedSampler;
		Sampler shadowSampler;

		BlendState blendState;
		Rasterizer rasterizer;
		DepthStencil depthStencil;

		UINT width;
		UINT height;

		PixelShader* pixelShaderOverride = nullptr;
		VertexShader* vertexShaderOverride = nullptr;

		GBuffer gBuffer;
		ShadowBuffer shadowBuffer;

		DirectX::XMFLOAT2 halton23[16] = {
			{0.5f, 0.333333f},
			{0.25f, 0.666667f},
			{0.75f, 0.111111f},
			{0.125f, 0.444444f},
			{0.625f, 0.777778f},
			{0.375f, 0.222222f},
			{0.875f, 0.555556f},
			{0.0625f, 0.888889f},
			{0.5625f, 0.037037f},
			{0.3125f, 0.370370f},
			{0.8125f, 0.703704f},
			{0.1875f, 0.148148f},
			{0.6875f, 0.481481f},
			{0.4375f, 0.814815f},
			{0.9375f, 0.259259f},
			{0.03125f, 0.592593f}
		};

		DirectX::XMFLOAT2 previousJitter = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 currentJitter = { 0.0f, 0.0f };
		float jitterScale = 0.4f;

		unsigned long long frameCount = 0;

	private:
		PostProcessing postProcessing;

		struct TAAData
		{
			DirectX::XMFLOAT2 jitter;
			DirectX::XMFLOAT2 previousJitter;
			BOOL useTAA = true;
			float padding[3];
		} taaData;

		struct CommonData
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
		} commonData;

		VertexConstantBuffer<CommonData> vcb { VS_CBUFFER_SLOT_COMMON };
		PixelConstantBuffer<CommonData> pcb { PS_CBUFFER_SLOT_COMMON };
		PixelConstantBuffer<TAAData> tab { 1u };
		PixelConstantBuffer<ShadowBuffer::ShadowData> shadowPixelBuffer { PS_CBUFFER_SLOT_SHADOW };
		PixelConstantBuffer<PostProcessing::PostProcessingData> ppb { 1u };

		static constexpr unsigned int SAMPLE_COUNT_DIRECTIONAL = 10u;
		static constexpr unsigned int SAMPLE_COUNT_SPOT = 4u;

		bool flipFlop = false;

	private:
		Skybox skybox = {};
		float skyboxSpeed = 0.005f;
		DirectX::XMFLOAT3 skyboxAngle = {};

		std::vector<Model> models = {};
	};
}
