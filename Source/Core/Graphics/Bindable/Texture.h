#pragma once
#include "Bindable.h"
#include <string>
#include <d3d11.h>
#include <stdexcept>
#include <wrl/client.h>


namespace Kaka
{
	class Texture : public Bindable
	{
	public:
		Texture(const UINT aSlot = 0u);
		~Texture() override = default;
		void LoadTextureFromModel(const Graphics& aGfx, const std::string& aFilePath);
		void LoadTextureFromPath(const Graphics& aGfx, const std::string& aFilePath);
		void LoadMaterialFromPaths(const Graphics& aGfx, const std::string& aFilePath1, const std::string& aFilePath2, const std::string& aFilePath3);
		void Bind(const Graphics& aGfx) override;
		void SetSlot(const UINT aSlot) { slot = aSlot; }
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetTextureAtIndex(size_t aIndex) { return pTextures[aIndex]; }
		void SetTextureAtIndex(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& aTexture, size_t aIndex) { pTextures[aIndex] = aTexture; }
		bool HasAlpha() const { return hasAlpha; }
	private:
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> pTextures;
		UINT slot;
		BOOL hasAlpha = FALSE;
	};

	struct Material
	{
		std::string name;
		Texture* textures[2u];
	};
}
