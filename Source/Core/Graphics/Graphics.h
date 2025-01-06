#pragma once
#include <DirectXMath.h>
#include <vector>
#include <External/Include/FileWatch/FileWatch.hpp>

#include "Core/Graphics/RenderTarget.h"
#include "Core/Graphics/GBuffer.h"
#include "Core/Graphics/RSMBuffer.h"
#include "Core/Graphics/ShadowBuffer.h"
#include "Core/Graphics/ShaderFactory.h"
#include "Core/Graphics/PostProcessing/PostProcessing.h"
#include "Core/Graphics/AntiAliasing/TemporalAntiAliasing.h"
#include "Core/Graphics/Lighting/DeferredLights.h"
#include "Core/Graphics/Lighting/IndirectLighting.h"
#include "Core/Graphics/Bindable/DepthStencil.h"
#include "Core/Graphics/Bindable/BlendState.h"
#include "Core/Graphics/Bindable/PixelShader.h"
#include "Core/Graphics/Bindable/Sampler.h"
#include "Core/Graphics/Bindable/Texture.h"
#include "Core/Graphics/Bindable/VertexShader.h"
#include "Core/Graphics/Bindable/Rasterizer.h"
#include "Core/Graphics/Bindable/CommonBuffer.h"
#include "Core/Graphics/Drawable/Skybox.h"
#include "Core/Graphics/Drawable/ModelData.h"
#include "Core/Graphics/Drawable/ModelRenderer.h"
#include "ECS/ECS.h"

#define KAKA_BG_COLOUR {0.1f, 0.2f, 0.3f, 1.0f}

namespace Kaka
{
	//class ECS;
	class Texture;
	class VertexShader;
	class PixelShader;
	class Camera;
	struct Mesh;

	struct RenderContext
	{
		float deltaTime;
		float totalTime;
		float fps;
	};

	//struct Transforms
	//{
	//	DirectX::XMMATRIX objectToWorld = {};
	//	DirectX::XMMATRIX objectToClip = {};
	//};

	//struct RenderData
	//{
	//	std::string filePath = "";
	//	MeshList* meshList = nullptr;
	//	VertexShader* vertexShader = nullptr;
	//	PixelShader* pixelShader = nullptr;
	//	DirectX::XMMATRIX* transform;
	//};

	//struct RenderPackage
	//{
	//	MeshList* meshList;
	//	DirectX::XMMATRIX* transform;
	//};

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
		friend class ModelRenderer;

	public:
		Graphics(HWND aHWnd, UINT aWidth, UINT aHeight);
		Graphics(const Graphics&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		~Graphics();
		void BeginFrame();
		void EndFrame();
		void DrawIndexed(UINT aCount);
		void DrawIndexedInstanced(UINT aCount, UINT aInstanceCount);
		void RegisterRenderPackage(const RenderData& aRenderData);
		void ClearRenderPackages();
		void BuildRenderQueue();
		//void RenderQueue();
		//void TempSetupModelRender();
		void Render(const RenderContext& aContext);
		void LoadModel(const std::string& aFilePath);

		// Camera
		DirectX::XMMATRIX GetProjection() const;
		DirectX::XMMATRIX GetJitteredProjection() const;
		void SetupCamera(const float aWidth, const float aHeight, const float aFoV = 80.0f, const float aNearZ = 0.5f, const float aFarZ = 5000.0f);
		void SetCamera(Camera& aCamera);
		DirectX::XMMATRIX GetCameraInverseView() const;
		bool IsPointInFrustum(const DirectX::XMFLOAT3& aPoint) const;
		bool IsBoundingBoxInFrustum(const DirectX::XMFLOAT3& aMin, const DirectX::XMFLOAT3& aMax) const;

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
		void StartShadows(Camera& aCamera, DirectX::XMFLOAT3 aLightDirection, const RSMBuffer& aBuffer);
		void StartShadows(Camera& aCamera, DirectX::XMFLOAT3 aLightDirection, const ShadowBuffer& aBuffer);
		void ResetShadows(Camera& aCamera);
		void BindShadows(const ShadowBuffer& aBuffer, UINT aSlot);
		void BindShadows(const RSMBuffer& aBuffer, UINT aSlot);
		void UnbindShadows(UINT aSlot);

		// Shader
		void SetPixelShaderOverride(const ePixelShaderType aType) { pixelShaderOverride = ShaderFactory::GetPixelShader(*this, aType); }
		void SetVertexShaderOverride(const eVertexShaderType aType) { vertexShaderOverride = ShaderFactory::GetVertexShader(*this, aType); }
		PixelShader* GetPixelShaderOverride() const { return pixelShaderOverride; }
		VertexShader* GetVertexShaderOverride() const { return vertexShaderOverride; }
		bool HasPixelShaderOverride() const { return pixelShaderOverride != nullptr; }
		bool HasVertexShaderOverride() const { return vertexShaderOverride != nullptr; }
		void ClearPixelShaderOverride() { pixelShaderOverride = nullptr; }
		void ClearVertexShaderOverride() { vertexShaderOverride = nullptr; }

		// ImGui
		void ShowImGui(const float aFPS);
		void ShowStatsWindow(const float aFPS);
		void ShowEntities(const std::vector<ECS::Entity*>& aEntities, EntityID& aOutSelectedEntity);
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
		ID3D11Buffer* CreateInstanceBuffer(const std::vector<DirectX::XMMATRIX>& instanceTransforms);

	private:
		UINT width;
		UINT height;
		UINT drawcallCount;
		uint64_t frameCount = 0;
		bool flipFlop = false;
		bool useReflectiveShadowMap = false;
		bool drawDebug = false;

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

		GBuffer gBuffer;
		ShadowBuffer shadowBuffer;
		CommonBuffer commonBuffer;
		RSMBuffer rsmBuffer;
		IndirectLighting indirectLighting;

		DeferredLights lightManager;

		PostProcessing postProcessing;
		TemporalAntiAliasing temporalAntiAliasing;

		PixelShader* pixelShaderOverride = nullptr;
		VertexShader* vertexShaderOverride = nullptr;

		Camera camera;
		Camera* currentCamera = nullptr;

	private:
		Skybox skybox = {};
		float skyboxSpeed = 0.005f;
		DirectX::XMFLOAT3 skyboxAngle = {};

		std::vector<ModelData> modelData;
		std::vector<RenderData> modelRenderData;

		ModelRenderer modelRenderer;
		RenderQueue renderQueue;
	};
}
