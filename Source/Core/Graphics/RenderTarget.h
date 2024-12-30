#pragma once
#include <wrl.h>
#include <d3d11.h>

namespace Kaka
{
	enum class eRenderTargetType
	{
		None,
		Default,
		PostProcessing,
		RSM,
		HistoryN1,
		HistoryN
	};

	struct RenderTarget
	{
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource;
	};
}