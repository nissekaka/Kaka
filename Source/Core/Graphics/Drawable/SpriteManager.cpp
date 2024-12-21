#include "stdafx.h"
//#include "SpriteManager.h"
//
//namespace Kaka
//{
//	void SpriteManager::Init(const Graphics& aGfx)
//	{
//		// Create vertex buffer
//
//		SpriteVertex vertices[4] = {
//			{-1.0f, -1.0f, 0.0f, 1.0f},
//			{-1.0f, 1.0f, 0.0f, 1.0f},
//			{1.0f, -1.0f, 0.0f, 1.0f},
//			{1.0f, 1.0f, 0.0f, 1.0f}
//		};
//
//		vertices[0].u = 0;
//		vertices[0].v = 1;
//		vertices[1].u = 0;
//		vertices[1].v = 0;
//		vertices[2].u = 1;
//		vertices[2].v = 1;
//		vertices[3].u = 1;
//		vertices[3].v = 0;
//
//		D3D11_BUFFER_DESC bufferDesc = {};
//		bufferDesc.ByteWidth = sizeof(SpriteVertex) * 4;
//		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//
//		D3D11_SUBRESOURCE_DATA subresourceData = {};
//		subresourceData.pSysMem = vertices;
//
//		ID3D11Device* device = aGfx.pDevice.Get();
//		HRESULT result = device->CreateBuffer(&bufferDesc, &subresourceData, &vertexBuffer);
//		if (FAILED(result))
//		{
//			assert(SUCCEEDED(result) && "Failed to create sprite vertex buffer!");
//		}
//
//		// Create instance buffer
//
//		D3D11_BUFFER_DESC instanceBufferDesc = {};
//
//		instanceBufferDesc.ByteWidth = sizeof(Sprite) * 8192;
//		instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
//		instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//		instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//		result = device->CreateBuffer(&instanceBufferDesc, nullptr, &instanceBuffer);
//		if (FAILED(result))
//		{
//			assert(SUCCEEDED(result) && "Failed to create sprite instance buffer!");
//			return;
//		}
//
//		// Create index buffer
//
//		const unsigned int indices[6] = {0, 1, 2, 2, 1, 3};
//
//		bufferDesc = {};
//		bufferDesc.ByteWidth = sizeof(unsigned int) * 6;
//		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//
//		subresourceData = {};
//		subresourceData.pSysMem = indices;
//
//		result = device->CreateBuffer(&bufferDesc, &subresourceData, &indexBuffer);
//		if (FAILED(result))
//		{
//			assert(SUCCEEDED(result) && "Failed to create sprite index buffer!");
//		}
//
//		// Create shaders
//
//		vertexShader = ShaderFactory::GetVertexShader(aGfx, L"shaders\\Sprite_VS.cso");
//		pixelShader = ShaderFactory::GetPixelShader(aGfx, L"shaders\\Sprite_PS.cso");
//
//		// Setup input layout
//
//		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
//		{
//			// Data from the vertex buffer
//			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}, // 3 floats
//			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}, // 2 floats
//
//			//// Data from the instance buffer
//			//{"INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
//			//{"INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
//			//{"INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
//			//{"INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
//			//{"INSTANCE_COLOUR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
//			//{"INSTANCE_TEX_BOUNDS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
//			//{"INSTANCE_TEX_REGION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
//		};
//
//		inputLayout.Init(aGfx, ied, vertexShader->GetBytecode());
//	}
//
//	void SpriteManager::Draw(Graphics& aGfx, SpriteBatch& aSpriteBatch)
//	{
//		aGfx.SetAlpha();
//
//		aSpriteBatch.data.texture->Bind(aGfx);
//
//		struct Transforms
//		{
//			DirectX::XMMATRIX objectToWorld;
//			DirectX::XMMATRIX objectToClip;
//		};
//
//		Transforms transforms;
//
//		const DirectX::XMMATRIX objectToWorld = GetSpriteTransform(aSpriteBatch.instances[0]);
//		DirectX::XMMATRIX objectToClip = objectToWorld * aGfx.GetCurrentCameraMatrix();
//		objectToClip = objectToClip * aGfx.GetProjection();
//		transforms.objectToWorld = objectToWorld;
//		transforms.objectToClip = objectToClip;
//
//		// Constant buffer
//		ID3D11Buffer* constantBuffer;
//		D3D11_BUFFER_DESC cbd;
//		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//		cbd.Usage = D3D11_USAGE_DYNAMIC;
//		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//		cbd.MiscFlags = 0u;
//		cbd.ByteWidth = sizeof(Transforms);
//		cbd.StructureByteStride = 0u;
//
//		D3D11_SUBRESOURCE_DATA csd = {};
//		csd.pSysMem = &transforms;
//		aGfx.pDevice->CreateBuffer(&cbd, &csd, &constantBuffer);
//
//		ID3D11DeviceContext* context = aGfx.pContext.Get();
//
//		aGfx.pContext->VSSetConstantBuffers(0, 1, &constantBuffer);
//
//		// Set vertex buffer
//		const unsigned int stride = sizeof(SpriteVertex);
//		const unsigned int offset = 0;
//		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
//
//		// Set index buffer
//		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
//
//		// Set primitive topology
//		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//		// Set instance buffer
//		context->IASetVertexBuffers(1, 1, &instanceBuffer, &stride, &offset);
//
//		// Set shaders
//		vertexShader->Bind(aGfx);
//		pixelShader->Bind(aGfx);
//
//		// Draw
//		aGfx.DrawIndexed(6);
//	}
//
//	//void SpriteManager::DrawBatch(Graphics& aGfx, const SpriteBatch* aSpriteBatch)
//	//{
//	//	aGfx.SetAlpha();
//
//	//	ID3D11DeviceContext* context = aGfx.pContext.Get();
//
//	//	// Set vertex buffer
//	//	const unsigned int vertexStride = sizeof(SpriteVertex);
//	//	const unsigned int vertexOffset = 0;
//	//	context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
//
//	//	// Set index buffer
//	//	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//	//	// Set primitive topology
//	//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//
//	//	const unsigned int instanceStride = sizeof(Sprite);
//	//	const unsigned int instanceOffset = 0;
//
//	//	// Set instance buffer
//	//	context->IASetVertexBuffers(1, 1, &instanceBuffer, &vertexStride, &vertexOffset);
//
//	//	// Draw
//	//	aGfx.DrawIndexedInstanced(6, aSpriteBatch->instances.size());
//	//}
//
//	DirectX::XMMATRIX SpriteManager::GetSpriteTransform(Sprite& aSprite) const
//	{
//		// TODO Rotates faster when scale is low, scale should only affect size
//		return DirectX::XMMatrixScaling(aSprite.transform.scale, aSprite.transform.scale, aSprite.transform.scale) *
//			DirectX::XMMatrixRotationRollPitchYaw(aSprite.transform.roll, aSprite.transform.pitch, aSprite.transform.yaw) *
//			DirectX::XMMatrixTranslation(aSprite.transform.x, aSprite.transform.y, aSprite.transform.z);
//	}
//}
