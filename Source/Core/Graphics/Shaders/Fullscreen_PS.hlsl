#include "common.hlsli"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D fullscreenTexture : register(t0);

float4 main(const PixelInput aInput) : SV_TARGET
{
    float4 colour = fullscreenTexture.Sample(linearSampler, aInput.texCoord).rgba;

    return colour;
}