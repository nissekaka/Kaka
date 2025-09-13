#include "../Common/common.hlsli"
#include "../Common/deferred_common.hlsli"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer TAABuffer : register(b1)
{
    float2 jitterOffset;
    float2 previousJitterOffset;
    bool useTAA;
};

Texture2D depthTexture : register(t0);
Texture2D currentTexture : register(t1);
Texture2D previousTexture : register(t2);
//Texture2D velocityTexture : register(t2);

float2 CameraReproject(float3 aPosition)
{
    const float4 screenPosition = mul(historyViewProjection, float4(aPosition, 1.0f));
    const float2 screenUV = screenPosition.xy / screenPosition.w;
    const float2 reprojectedUV = screenUV * float2(0.5f, -0.5f) + 0.5f;
    return reprojectedUV;
}

float4 main(const PixelInput aInput) : SV_TARGET
{
    if (!useTAA)
    {
        return currentTexture.Sample(fullscreenSampler, aInput.texCoord);
    }
    
    const float depth = depthTexture.Sample(fullscreenSampler, aInput.texCoord).r;
    const float3 worldPos = ReconstructWorldPosition(aInput.texCoord, depth, inverseViewProjection);

    const float2 prevUV = CameraReproject(worldPos);
    
    const float3 currentColour = currentTexture.Sample(fullscreenSampler, aInput.texCoord).rgb;
    const float3 previousColour = previousTexture.Sample(fullscreenSampler, prevUV).rgb;

    //return float4(previousColour, 1.0f);
    
    float3 minColour = 9999.0, maxColour = -9999.0;
 
	// Sample a 3x3 neighborhood to create a box in color space
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            const float3 colour = currentTexture.Sample(fullscreenSampler, aInput.texCoord + float2(x, y) / resolution);
            minColour = min(minColour, colour);
            maxColour = max(maxColour, colour);
        }
    }
 
    const float3 previousColourClamped = clamp(previousColour, minColour, maxColour);

    const float3 output = currentColour * 0.1f + previousColourClamped * 0.9f;

    return float4(output, 1.0f);
}