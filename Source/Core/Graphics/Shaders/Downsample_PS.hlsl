#include "common.hlsli"

cbuffer Bloom : register(b1)
{
    float bloomBlending;
    float bloomThreshold;
    int uvScale;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D fullscreenTexture : register(t0);

float4 main(PixelInput aInput) : SV_TARGET
{
    float4 returnValue;
	// ddx and ddy measures how much a parameter changes in x and y-direction per pixel
	// could have used
    aInput.texCoord *= uvScale;
    const float2 pixelOffset = float2(ddx(aInput.texCoord.x), ddy(aInput.texCoord.y));
	// Could have done one sample in the middle
	// But that results in some artifacts. This pattern gives a much smoother result
    if (aInput.texCoord.x < 0.001f || aInput.texCoord.x > 0.999f || aInput.texCoord.y < 0.001f || aInput.texCoord.y > 0.999f)
    {
        returnValue.rgb = 0.0f;
        returnValue.a = 0.0f;
        return returnValue;
    }
    const float3 p00 = fullscreenTexture.Sample(linearSampler, aInput.texCoord + pixelOffset * float2(-0.5f, -0.5f)).rgb;
    const float3 p01 = fullscreenTexture.Sample(linearSampler, aInput.texCoord + pixelOffset * float2(-0.5f, 0.5f)).rgb;
    const float3 p10 = fullscreenTexture.Sample(linearSampler, aInput.texCoord + pixelOffset * float2(0.5f, -0.5f)).rgb;
    const float3 p11 = fullscreenTexture.Sample(linearSampler, aInput.texCoord + pixelOffset * float2(0.5f, 0.5f)).rgb;

    returnValue.rgb = bloomThreshold * (p00 + p01 + p10 + p11);
    returnValue.a = 1.0f;

	return returnValue;
}