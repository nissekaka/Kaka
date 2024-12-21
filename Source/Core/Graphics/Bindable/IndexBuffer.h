#pragma once
#include "Bindable.h"


namespace Kaka
{
	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(const Graphics& aGfx, const std::vector<unsigned short>& aIndices);
		void Init(const Graphics& aGfx, const std::vector<unsigned short>& aIndices);
		void Bind(const Graphics& aGfx) override;
		UINT GetCount() const;

	protected:
		UINT count = {};
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	};
}
