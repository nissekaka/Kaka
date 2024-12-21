
#include "GBuffer.hlsli"
#include "RSM.hlsli"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer TAABuffer : register(b1)
{
    bool useTAA;
    float padding2;
    float2 jitterOffset;
    float2 previousJitterOffset;
};

float4 main(const PixelInput aInput) : SV_TARGET
{
    const float2 uv = aInput.texCoord;

    const float3 albedo = gColourTex.Sample(defaultSampler, uv).rgb;
    const float3 worldPosition = gWorldPositionTex.Sample(linearSampler, uv).rgb;
    const float3 worldNormal = normalize(2.0f * gNormalTex.Sample(linearSampler, uv).xyz - 1.0f);

    const float4 lightProjectedPositionTemp = mul(lightCameraTransform, float4(worldPosition, 1.0f));
    const float3 lightProjectedPosition = lightProjectedPositionTemp.xyz / lightProjectedPositionTemp.w;

    const float2 sampleUV = 0.5f + float2(0.5f, -0.5f) * (lightProjectedPosition.xy);

    float3 rsm = IndirectLighting(sampleUV, worldNormal, worldPosition,
    rMax, sampleCount, rsmIntensity, aInput.position.xy) * albedo;

    return float4(rsm, 1.0f);
}
