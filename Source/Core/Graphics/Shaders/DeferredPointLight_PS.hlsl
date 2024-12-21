#include "common.hlsli"
#include "deferred_common.hlsli"
#include "GBuffer.hlsli"
#include "PBRFunctions.hlsli"

cbuffer PointlightData : register(b2)
{
    float3 lightPosition;
    float lightIntensity;
    float3 lightColour;
    float lightRadius;
    bool lightIsActive;
    float3 padding;
};

float4 main(DeferredVertexToPixel aInput) : SV_TARGET
{
    const float2 uv = aInput.position.xy / resolution.xy;
    const float3 worldPosition = gWorldPositionTex.Sample(linearSampler, uv).rgb;

    const float3 albedo = gColourTex.Sample(defaultSampler, uv).rgb;
    const float3 normal = normalize(2.0f * gNormalTex.Sample(linearSampler, uv).xyz - 1.0f);
    const float4 material = gMaterialTex.Sample(linearSampler, uv);

    if (material.a == 0.0f)
    {
        discard;
    }

    const float metalness = material.b;
    const float roughness = material.g;

    const float3 specular = lerp((float3) 0.04f, albedo.rgb, metalness);
    const float3 colour = lerp((float3) 0.0f, albedo.rgb, 1 - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - worldPosition.xyz);

    const float3 pointLight = EvaluatePointLight(colour, specular, normal,
                                                 roughness, lightColour, lightIntensity,
                                                 lightRadius, lightPosition, toEye.xyz, worldPosition.xyz);
    
    float3 finalColour = pointLight;

    return float4(finalColour.rgb, 1.0f);
}
