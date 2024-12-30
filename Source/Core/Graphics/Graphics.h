#pragma once
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <memory>
#include <vector>
#include <External/Include/FileWatch/FileWatch.hpp>

#include "Core/Graphics/RenderTarget.h"

#include "Core/Graphics/GraphicsConstants.h"
#include "Core/Graphics/GBuffer.h"
#include "Core/Graphics/RSMBuffer.h"
#include "Core/Graphics/ShadowBuffer.h"
#include "Core/Graphics/ShaderFactory.h"
#include "Core/Graphics/PostProcessing/PostProcessing.h"
#include "Core/Graphics/AntiAliasing/TemporalAntiAliasing.h"
#include "Core/Graphics/Lighting/DeferredLights.h"
#include "Core/Graphics/Lighting/IndirectLighting.h"
#include "Core/Graphics/Bindable/ConstantBuffers.h"
#include "Core/Graphics/Bindable/DepthStencil.h"
#include "Core/Graphics/Bindable/BlendState.h"
#include "Core/Graphics/Bindable/CommonBuffer.h"
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

	struct RenderContext
	{
		const float deltaTime;
		const float totalTime;
		const float fps;
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
		void Render(const RenderContext& aContext);

		// Camera
		DirectX::XMMATRIX GetProjection() const;
		DirectX::XMMATRIX GetJitteredProjection() const;
		void SetupCamera(const float aWidth, const float aHeight, const float aFoV = 80.0f, const float aNearZ = 0.5f, const float aFarZ = 5000.0f);
		void SetCamera(Camera& aCamera);
		DirectX::XMMATRIX GetCameraInverseView() const;

		// Render info
		DirectX::XMFLOAT2 GetCurrentResolution() const;
		UINT GetWidth() const;
		UINT GetHeight() const;
		UINT GetDrawcallCount() const;

		// Bindables
		void SetRenderTarget(eRenderTargetType aRenderTargetType, ID3D11DepthStencilView* aDepth) const;
		void SetBlendState(eBlendStates aBlendState);
		void SetDepthStencilState(eDepthStencilStates aDepthStencilState);
		void SetRasterizerState(eRasterizerStates aRasterizerState);

		// Shadows
		void StartShadows(Camera& aCamera, const DirectX::XMFLOAT3 aLightDirection, const ShadowBuffer& aBuffer, UINT aSlot);
		void StartShadows(Camera& aCamera, const DirectX::XMFLOAT3 aLightDirection, const RSMBuffer& aBuffer, UINT aSlot);
		void ResetShadows(Camera& aCamera);
		void BindShadows(const ShadowBuffer& aBuffer, UINT aSlot);
		void BindShadows(const RSMBuffer& aBuffer, UINT aSlot);
		void UnbindShadows(UINT aSlot);

		// Shader
		void SetPixelShaderOverride(const std::wstring& aFileName) { pixelShaderOverride = ShaderFactory::GetPixelShader(*this, aFileName); }
		void SetVertexShaderOverride(const std::wstring& aFileName) { vertexShaderOverride = ShaderFactory::GetVertexShader(*this, aFileName); }
		PixelShader* GetPixelShaderOverride() const { return pixelShaderOverride; }
		VertexShader* GetVertexShaderOverride() const { return vertexShaderOverride; }
		bool HasPixelShaderOverride() const { return pixelShaderOverride != nullptr; }
		bool HasVertexShaderOverride() const { return vertexShaderOverride != nullptr; }
		void ClearPixelShaderOverride() { pixelShaderOverride = nullptr; }
		void ClearVertexShaderOverride() { vertexShaderOverride = nullptr; }

		// ImGui
		void ShowImGui(const float aFPS);
		void ShowStatsWindow(const float aFPS);
		void EnableImGui();
		void DisableImGui();
		bool IsImGuiEnabled() const;

	private:
		bool imGuiEnabled = true;
		bool showImGui = true;
		bool showStatsWindow = true;
		bool drawLightDebug = false;
	
	public:
		void UpdateLights(const float aDeltaTime);
		void ProcessFileChangeEngine(const std::wstring& aPath, filewatch::Event aEvent);

	private:
		struct FrustumPlanes
		{
			DirectX::XMFLOAT4 planes[6];
		};

		FrustumPlanes ExtractFrustumPlanes() const;

	public:
		bool IsBoundingBoxInFrustum(const DirectX::XMFLOAT3& aMin, const DirectX::XMFLOAT3& aMax) const;

	private:
		UINT width;
		UINT height;
		UINT drawcallCount;
		unsigned long long frameCount = 0;
		bool flipFlop = false;
		bool useReflectiveShadowMap = true;

		filewatch::FileWatch<std::wstring> shaderFileWatcher;

		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pDefaultTarget;

		Sampler defaultSampler;
		Sampler linearSampler;
		Sampler pointClampedSampler;
		Sampler shadowSampler;

		BlendState blendState;
		Rasterizer rasterizer;
		DepthStencil depthStencil;

		PixelShader* pixelShaderOverride = nullptr;
		VertexShader* vertexShaderOverride = nullptr;

		GBuffer gBuffer;
		ShadowBuffer shadowBuffer;
		CommonBuffer commonBuffer;
		RSMBuffer rsmBuffer;
		IndirectLighting indirectLighting;

		DeferredLights lightManager;

		PostProcessing postProcessing;
		TemporalAntiAliasing temporalAntiAliasing;

		Camera camera;
		Camera* currentCamera = nullptr;

	private:
		Skybox skybox = {};
		float skyboxSpeed = 0.005f;
		DirectX::XMFLOAT3 skyboxAngle = {};

		std::vector<Model> models = {};
	};
}
