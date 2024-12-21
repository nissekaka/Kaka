#include "common.hlsli"
#include "deferred_common.hlsli"

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

	// Use history view-projection matrix to project onto previous camera's screen space
    const float3 worldPosition = worldPositionTexture.Sample(linearSampler, aInput.texCoord).rgb;

    float2 reprojectedUV = aInput.texCoord;

    // If the world position is valid, reproject the UV
    if (length(worldPosition) > 0.0f)
    {
        reprojectedUV = CameraReproject(worldPosition);
    }

    //const float2 motionVector = velocityTexture.Sample(linearSampler, aInput.texCoord).xy;
    //const float2 reprojectedUV = (aInput.texCoord - motionVector); // / resolution;

    const float3 currentColour = currentTexture.Sample(linearSampler, aInput.texCoord).rgb;
    const float3 previousColour = previousTexture.Sample(linearSampler, reprojectedUV).rgb;

    //return float4(previousColour, 1.0f);

    // Arbitrary out of range numbers
    float3 minColour = 9999.0, maxColour = -9999.0;
 
	// Sample a 3x3 neighborhood to create a box in color space
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            const float3 colour = currentTexture.Sample(linearSampler, aInput.texCoord + float2(x, y) / resolution); // Sample neighbor
            minColour = min(minColour, colour); // Take min and max
            maxColour = max(maxColour, colour);
        }
    }
 
	// Clamp previous color to min/max bounding box
    const float3 previousColourClamped = clamp(previousColour, minColour, maxColour);

    float3 output = currentColour * 0.1f + previousColourClamped * 0.9f;

    //return float4(output, 1.0f);

    float3 antialiased = previousColourClamped.rgb;

    float2 off = float2(1.0f / resolution.x, 1.0f / resolution.y);
    float3 in0 = output; //    currentTexture.Sample(linearSampler, uv).rgb;

    antialiased = lerp(antialiased * antialiased, in0 * in0, 0.5f);
    antialiased = sqrt(antialiased);

    float3 in1 = currentTexture.Sample(pointSampler, aInput.texCoord + float2(+off.x, 0.0)).xyz;
    float3 in2 = currentTexture.Sample(pointSampler, aInput.texCoord + float2(-off.x, 0.0)).xyz;
    float3 in3 = currentTexture.Sample(pointSampler, aInput.texCoord + float2(0.0, +off.y)).xyz;
    float3 in4 = currentTexture.Sample(pointSampler, aInput.texCoord + float2(0.0, -off.y)).xyz;
    float3 in5 = currentTexture.Sample(pointSampler, aInput.texCoord + float2(+off.x, +off.y)).xyz;
    float3 in6 = currentTexture.Sample(pointSampler, aInput.texCoord + float2(-off.x, +off.y)).xyz;
    float3 in7 = currentTexture.Sample(pointSampler, aInput.texCoord + float2(+off.x, -off.y)).xyz;
    float3 in8 = currentTexture.Sample(pointSampler, aInput.texCoord + float2(-off.x, -off.y)).xyz;

    minColour = min(min(min(in0, in1), min(in2, in3)), in4);
    maxColour = max(max(max(in0, in1), max(in2, in3)), in4);
    minColour = lerp(minColour, min(min(min(in5, in6), min(in7, in8)), minColour), 0.5);
    maxColour = lerp(maxColour, max(max(max(in5, in6), max(in7, in8)), maxColour), 0.5);

    antialiased = clamp(antialiased, minColour, maxColour);

    return float4(antialiased, 1.0f);
}