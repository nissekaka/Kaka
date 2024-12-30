#include "RSM.hlsli"

Texture2D currentDirectional : register(t0);
Texture2D previousDirectional : register(t1);
Texture2D worldPositionTexture : register(t2);
Texture2D currentSpot : register(t3);

cbuffer TAABuffer : register(b1)
{
    bool useTAA;
    float padding2;
    float2 jitterOffset;
    float2 previousJitterOffset;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float2 CameraReproject(float3 aPosition)
{
    // Transform screen space position to UV and sample the previous texture
    float4 screenPosition = mul(historyViewProjection, float4(aPosition, 1.0f));
    const float2 screenUV = screenPosition.xy / screenPosition.w;
    float2 reprojectedUV = (screenUV * float2(0.5f, -0.5f) + 0.5f);
    return reprojectedUV;
}

// YUV-RGB conversion routine from Hyper3D
float3 encodePalYuv(float3 rgb)
{
    rgb = pow(rgb, float3(2.0, 2.0, 2.0)); // Gamma correction
    return float3(
        dot(rgb, float3(0.299, 0.587, 0.114)),
        dot(rgb, float3(-0.14713, -0.28886, 0.436)),
        dot(rgb, float3(0.615, -0.51499, -0.10001))
    );
}

float3 decodePalYuv(float3 yuv)
{
    float3 rgb = float3(
        dot(yuv, float3(1., 0., 1.13983)),
        dot(yuv, float3(1., -0.39465, -0.58060)),
        dot(yuv, float3(1., 2.03211, 0.))
    );
    return pow(rgb, float3(0.5, 0.5, 0.5)); // Gamma correction
}

float4 main(const PixelInput aInput) : SV_TARGET
{
    const float3 worldPosition = worldPositionTexture.Sample(linearSampler, aInput.texCoord).rgb;

	// Use history view-projection matrix to project onto previous camera's screen space
    float2 reprojectedUV = aInput.texCoord;

    // If the world position is valid, reproject the UV
    if (length(worldPosition) > 0.0f)
    {
        reprojectedUV = CameraReproject(worldPosition);
    }

    const float3 currentColour = currentDirectional.Sample(linearSampler, aInput.texCoord).rgb;
    const float3 previousColour = previousDirectional.Sample(linearSampler, reprojectedUV).rgb;

    // Arbitrary out of range numbers
    float3 minColour = 9999.0, maxColour = -9999.0;
 
	// Sample a 3x3 neighborhood to create a box in color space
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            const float3 colour = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(x, y) / resolution); // Sample neighbor
            minColour = min(minColour, colour); // Take min and max
            maxColour = max(maxColour, colour);
        }
    }
 
	// Clamp previous color to min/max bounding box
    const float3 previousColourClamped = clamp(previousColour, minColour, maxColour);

    const float3 output = currentColour * 0.2f + previousColourClamped * 0.8f;

    float3 antialiased = previousColourClamped;

    float2 off = float2(1.0f / resolution.x, 1.0f / resolution.y);
    float3 in0 = currentColour;

    antialiased = lerp(antialiased * antialiased, in0 * in0, 0.5f);
    antialiased = sqrt(antialiased);

    float3 in1 = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(+off.x, 0.0)).xyz;
    float3 in2 = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(-off.x, 0.0)).xyz;
    float3 in3 = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(0.0, +off.y)).xyz;
    float3 in4 = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(0.0, -off.y)).xyz;
    float3 in5 = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(+off.x, +off.y)).xyz;
    float3 in6 = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(-off.x, +off.y)).xyz;
    float3 in7 = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(+off.x, -off.y)).xyz;
    float3 in8 = currentDirectional.Sample(linearSampler, aInput.texCoord + float2(-off.x, -off.y)).xyz;

    antialiased = encodePalYuv(antialiased);
    in0 = encodePalYuv(in0);
    in1 = encodePalYuv(in1);
    in2 = encodePalYuv(in2);
    in3 = encodePalYuv(in3);
    in4 = encodePalYuv(in4);
    in5 = encodePalYuv(in5);
    in6 = encodePalYuv(in6);
    in7 = encodePalYuv(in7);
    in8 = encodePalYuv(in8);

    minColour = min(min(min(in0, in1), min(in2, in3)), in4);
    maxColour = max(max(max(in0, in1), max(in2, in3)), in4);
    minColour = lerp(minColour, min(min(min(in5, in6), min(in7, in8)), minColour), 0.5);
    maxColour = lerp(maxColour, max(max(max(in5, in6), max(in7, in8)), maxColour), 0.5);

    antialiased = clamp(antialiased, minColour, maxColour);

    antialiased = decodePalYuv(antialiased);

    return float4(antialiased, 1.0f);
}
