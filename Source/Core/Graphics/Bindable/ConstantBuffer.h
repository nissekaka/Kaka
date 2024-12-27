#pragma once
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct ID3D11Buffer;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct D3D11_BUFFER_DESC;

namespace Kaka
{
	class KonstantBuffer
	{
	public:
		KonstantBuffer();
		KonstantBuffer(const UINT aSlot);
		~KonstantBuffer();

		ComPtr<ID3D11Buffer>& GetBuffer();

		HRESULT Init(ComPtr<ID3D11Device> aDevice, const UINT aSize, const UINT aSlot);

		void MapBuffer(void* aData, const UINT aSize, ID3D11DeviceContext* aContext);

		void BindForPS(ID3D11DeviceContext* aContext);
		void BindForVS(ID3D11DeviceContext* aContext);
		//void BindForPS(const UINT aSlot, ID3D11DeviceContext* aContext);
		//void BindForVS(const UINT aSlot, ID3D11DeviceContext* aContext);
	private:
		ComPtr<ID3D11Buffer> buffer;
		UINT slot;
	};
}