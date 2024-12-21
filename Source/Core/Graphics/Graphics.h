#pragma once
#include <wrl.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <memory>
#include <vector>
#include "GBuffer.h"
#include "RSMBuffer.h"
#include "ShaderFactory.h"
#include <External/Include/FileWatch/FileWatch.hpp>

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
		WaterReflect,
		PostProcessing,
		IndirectLight,
		RSMFullscaleSpot,
		HistoryN1,
		HistoryN
	};

	enum class eBlendStates
	{
		Disabled,
		Alpha,
		Additive,
		TransparencyBlend,
		VFX,
		Count,
	};

	enum class eDepthStencilStates
	{
		Normal,
		ReadOnlyGreater,
		ReadOnlyLessEqual,
		ReadOnlyEmpty,
		Count,
	};

	enum class eRasterizerStates
	{
		BackfaceCulling,
		FrontfaceCulling,
		NoCulling,
		Count,
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
		void SetCamera(Camera& aCamera);
		DirectX::XMMATRIX GetCameraInverseView() const;
		UINT GetDrawcallCount() const;
		void SetRenderTarget(eRenderTargetType aRenderTargetType, const bool aUseDepth = true) const;
		void SetRenderTarget(eRenderTargetType aRenderTargetType, ID3D11DepthStencilView* aDepth) const;
		void SetRenderTargetShadow(const RSMBuffer& aBuffer) const;
		void ApplyProjectionJitter();

		void HandleBloomScaling(PostProcessing& aPostProcessor, ID3D11ShaderResourceView* aResource);

		void SetBlendState(eBlendStates aBlendState);
		void SetDepthStencilState(eDepthStencilStates aDepthStencilState);
		void SetRasterizerState(eRasterizerStates aRasterizerState);

		bool CreateBlendStates();
		bool CreateDepthStencilStates();
		bool CreateRasterizerStates();

		void BindWaterReflectionTexture();
		void BindPostProcessingTexture();
		void BindBloomDownscaleTexture(const int aIndex);
		DirectX::XMFLOAT2 GetCurrentResolution() const;

		void StartShadows(Camera& aCamera, const DirectX::XMFLOAT3 aLightDirection, const RSMBuffer& aBuffer, UINT aSlot);
		void ResetShadows(Camera& aCamera);
		void BindShadows(const RSMBuffer& aBuffer, UINT aSlot);
		void UnbindShadows(UINT aSlot);

		void SetPixelShaderOverride(const std::wstring& aFileName) { pixelShaderOverride = ShaderFactory::GetPixelShader(*this, aFileName); }
		void SetVertexShaderOverride(const std::wstring& aFileName) { vertexShaderOverride = ShaderFactory::GetVertexShader(*this, aFileName); }
		PixelShader* GetPixelShaderOverride() const { return pixelShaderOverride; }
		VertexShader* GetVertexShaderOverride() const { return vertexShaderOverride; }
		bool HasPixelShaderOverride() const { return pixelShaderOverride != nullptr; }
		bool HasVertexShaderOverride() const { return vertexShaderOverride != nullptr; }
		void ClearPixelShaderOverride() { pixelShaderOverride = nullptr; }
		void ClearVertexShaderOverride() { vertexShaderOverride = nullptr; }

	public:
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

		Camera* camera = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;

		RenderTarget renderWaterReflect;
		RenderTarget postProcessing;

		RenderTarget indirectLight;
		RenderTarget indirectLightN;
		RenderTarget indirectLightN1;
		//RenderTarget rsmFullscaleSpot;

		RenderTarget historyN;
		RenderTarget historyN1;

		DirectX::XMMATRIX historyViewProjection;

		std::vector<RenderTarget> bloomDownscale = {};

		struct DownSampleBuffer
		{
			float bloomBlending = 0.0f;
			float bloomThreshold = 0.1f;
			int uvScale = 2;
			float padding;
		} bb;

		int bloomDivideFactor = 2;
		bool usePostProcessing = true;
		int bloomSteps = 5;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pDefaultTarget;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pDepthShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepth;

		Microsoft::WRL::ComPtr<ID3D11BlendState> pBlendStates[(int)eBlendStates::Count];
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthStencilStates[(int)eDepthStencilStates::Count];
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerStates[(int)eRasterizerStates::Count];

		Microsoft::WRL::ComPtr<ID3D11SamplerState> pDefaultSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pLinearSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pPointClampedSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pVFXSampler;

		UINT width;
		UINT height;

		PixelShader* pixelShaderOverride = nullptr;
		VertexShader* vertexShaderOverride = nullptr;

		GBuffer gBuffer;
		RSMBuffer directionalLightRSMBuffer;
		std::vector<RSMBuffer> spotLightRSMBuffer;

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

		DirectX::XMFLOAT2 previousJitter = {0.0f, 0.0f};
		DirectX::XMFLOAT2 currentJitter = {0.0f, 0.0f};
		float jitterScale = 0.4f;

		unsigned long long frameCount = 0;

	private:
	};
}
