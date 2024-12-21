//#pragma once
//#include "Drawable.h"
//#include "Sprite.h"
//
//namespace Kaka
//{
//	class SpriteManager
//	{
//	public:
//		SpriteManager() = default;
//		~SpriteManager() = default;
//
//		void Init(const Graphics& aGfx);
//		void Draw(Graphics& aGfx, SpriteBatch& aSpriteBatch);
//		//void DrawBatch(Graphics& aGfx, const SpriteBatch* aSpriteBatch);
//		DirectX::XMMATRIX GetSpriteTransform(Sprite& aSprite) const;
//
//	private:
//		std::vector<SpriteBatch*> spriteBatches;
//
//		ID3D11Buffer* instanceBuffer = nullptr;
//
//		ID3D11Buffer* vertexBuffer = nullptr;
//		ID3D11Buffer* indexBuffer = nullptr;
//
//		VertexShader* vertexShader = nullptr;
//		PixelShader* pixelShader = nullptr;
//
//		InputLayout inputLayout;
//	};
//}
