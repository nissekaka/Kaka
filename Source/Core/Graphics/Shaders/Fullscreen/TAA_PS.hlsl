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

Texture2D currentTexture : register(t0);
Texture2D previousTexture : register(t1);
Texture2D worldPositionTexture : register(t2);
//Texture2D depthTexture : register(t3);
//Texture2D velocityTexture : register(t2);

float2 CameraReproject(float3 aPosition)
{
    // Transform screen space position to UV and sample the previous texture
    const float4 screenPosition = mul(historyViewProjection, float4(aPosition, 1.0f));
    const float2 screenUV = screenPosition.xy / screenPosition.w;
    const float2 reprojectedUV = screenUV * float2(0.5f, -0.5f) + 0.5f;
    return reprojectedUV;
}

float4 main(const PixelInput aInput) : SV_TARGET
{
    if (!useTAA)
    {
        return currentTexture.Sample(pointSampler, aInput.texCoord);
    }

    // Fetch world position
    const float3 worldPosition = worldPositionTexture.Sample(pointSampler, aInput.texCoord).rgb;

    float2 reprojectedUV = aInput.texCoord;

    // If the world position is valid, reproject into history buffer
    if (length(worldPosition) > 0.0f)
    {
        reprojectedUV = CameraReproject(worldPosition);

        // Apply jitter correction (remove previous jitter, add current jitter)
        float2 jitterDelta = (jitterOffset - previousJitterOffset) / resolution;
        reprojectedUV += jitterDelta;
    }

    // Prevent UV wrapping artifacts (top of screen sampling bottom, etc.)
    reprojectedUV = saturate(reprojectedUV);

    // Sample current and history
    const float3 currentColour  = currentTexture.Sample(pointSampler, aInput.texCoord).rgb;
    const float3 previousColour = previousTexture.Sample(linearSampler, reprojectedUV).rgb;

    // Build color bounding box from 3x3 neighborhood
    float3 minColour = 9999.0, maxColour = -9999.0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            const float2 offset = float2(x, y) / resolution;
            const float3 neighbour = currentTexture.Sample(linearSampler, aInput.texCoord + offset).rgb;
            minColour = min(minColour, neighbour);
            maxColour = max(maxColour, neighbour);
        }
    }

    // Clamp history color to neighborhood bounds
    const float3 previousColourClamped = clamp(previousColour, minColour, maxColour);

    // Temporal accumulation (blend current with history)
    float3 output = lerp(currentColour, previousColourClamped, 0.9f);

    return float4(output, 1.0f);
}