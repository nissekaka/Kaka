#include "deferred_common.hlsli"
#include "GBuffer.hlsli"
#include "PBRFunctions.hlsli"
#include "Shadows.hlsli"
#include "Volumetric.hlsli"

TextureCube daySkyTex : register(t12);
TextureCube nightSkyTex : register(t13);

cbuffer DirectionalLight : register(b1)
{
    float3 directionalLightDirection;
    float directionalLightIntensity;
    float3 directionalLightColour;
    float ambientLightIntensity;
    float4x4 directionalLightCameraTransform;
    uint numberOfVolumetricSteps;
    float volumetricScattering;
    float volumetricIntensity;
    float padding;
    float4 shadowColour;
    float4 ambianceColour;
};

float4 main(DeferredVertexToPixel aInput) : SV_TARGET
{
    const float2 uv = aInput.position.xy / resolution.xy;
    const float3 worldPosition = gWorldPositionTex.Sample(linearSampler, uv).rgb;
    const float3 albedo = gColourTex.Sample(defaultSampler, uv).rgb;
    const float4 ambientOcclusionAndCustom = gAmbientOcclusionTex.Sample(linearSampler, uv).rgba;
    
    const float3 normal = normalize(2.0f * gNormalTex.Sample(linearSampler, uv).xyz - 1.0f);
    const float4 material = gMaterialTex.Sample(linearSampler, uv);

    const float roughness = material.g;
    const float metalness = material.b;

    const float3 specular = lerp((float3) 0.04f, albedo.rgb, metalness);
    const float3 colour = lerp((float3) 0.0f, albedo.rgb, 1.0f - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - worldPosition);

    const float4 lightProjectedPositionTemp = mul(directionalLightCameraTransform, float4(worldPosition, 1.0f));
    const float3 lightProjectedPosition = lightProjectedPositionTemp.xyz / lightProjectedPositionTemp.w;

    float shadowFactor = Shadow(lightProjectedPosition, directionalLightShadowMap) + shadowColour.w;
    shadowFactor = saturate(shadowFactor);

    float3 dirLightDir = directionalLightDirection;
    dirLightDir.x = -dirLightDir.x;

    const float3 directionalLight = EvaluateDirectionalLight(colour, specular, normal,
    roughness, directionalLightColour, -dirLightDir, toEye) * directionalLightIntensity;

    const float3 volumetric = Volumetric(worldPosition, cameraPosition.xyz, numberOfVolumetricSteps,
                                         uv, resolution.xy, directionalLightCameraTransform,
                                         directionalLightShadowMap, linearSampler, dirLightDir,
                                         directionalLightColour, volumetricScattering) * volumetricIntensity;

    float3 finalColour = directionalLight * shadowFactor;

    if (shadowFactor < 1.0f)
    {
        finalColour *= shadowColour.rgb;
    }

    const float3 lightDir = normalize(directionalLightDirection);
    const float dotProduct = dot(lightDir, float3(0.0f, 1.0f, 0.0f));
    const float blendFactor = (dotProduct + 1.0) / 2.0f;

    const float3 ambiance = EvaluateAmbianceDynamicSky(linearSampler, daySkyTex, nightSkyTex, blendFactor,
    normal, ambientOcclusionAndCustom.gba, toEye, roughness, ambientOcclusionAndCustom.r, colour, specular);

    const float3 constantAmbiance = colour * ambianceColour.rgb * ambianceColour.w;

    return saturate(float4(finalColour + constantAmbiance + ambiance * ambientLightIntensity + volumetric, 1.0f));
}