#pragma once
#include "Bindable.h"


namespace Kaka
{
	class DepthStencil : public Bindable
	{
	public:
		enum class Mode
		{
			Off,
			Normal,
			Write,
			Mask,
			DepthOff,
			DepthReversed,
			DepthFirst // For skybox render
		};

		DepthStencil() = default;
		DepthStencil(const Graphics& aGfx, const Mode aMode);
		void Init(const Graphics& aGfx, const Mode aMode);
		void Bind(const Graphics& aGfx) override;

	private:
		Mode mode = Mode::Off;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pStencil;
	};
}
