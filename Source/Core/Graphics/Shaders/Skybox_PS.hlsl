#include "common.hlsli"

TextureCube daySkyTex : register(t12);
TextureCube nightSkyTex : register(t13);

cbuffer DirectionalLight : register(b1)
{
    float3 directionalLightDirection;
    float directionalLightIntensity;
    float3 directionalLightColour;
    float ambientLightIntensity;
    float4x4 directionalLightCameraTransform;
};

struct PixelInput
{
    float3 worldPosition : POSITION;
    float4 position : SV_POSITION;
};

float4 main(const PixelInput aInput) : SV_TARGET
{
    const float3 day = daySkyTex.Sample(linearSampler, aInput.worldPosition).rgb;
    const float3 night = nightSkyTex.Sample(linearSampler, aInput.worldPosition).rgb;
    const float3 lightDir = normalize(directionalLightDirection);
    const float dotProduct = dot(lightDir, float3(0.0f, 1.0f, 0.0f));
    const float blendFactor = (dotProduct + 1.0) / 2.0f;
    const float4 colour = float4(lerp(day, night, blendFactor), 1.0f);
    
    return colour;
}