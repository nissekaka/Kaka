#pragma once
#include "Bindable.h"
#include <string>


namespace Kaka
{
	class CubeTexture : public Bindable
	{
	public:
		CubeTexture(const UINT aSlot = 0u);
		~CubeTexture() override = default;
		void LoadTextures(const Graphics& aGfx, const std::string& aFolderPath);
		void Bind(const Graphics& aGfx) override;

	private:
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> pTextures;
		UINT slot;
	};
}
