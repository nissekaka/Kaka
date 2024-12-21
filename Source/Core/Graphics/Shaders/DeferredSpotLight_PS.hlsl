#include "PBRFunctions.hlsli"
//#include "RSM.hlsli"
#include "deferred_common.hlsli"
#include "GBuffer.hlsli"
#include "Shadows.hlsli"
#include "Volumetric.hlsli"

Texture2D flashlightTex : register(t5);

cbuffer DirectionalLight : register(b1)
{
    float3 directionalLightDirection;
    float directionalLightIntensity;
    float3 directionalLightColour;
    float ambientLightIntensity;
    float4x4 directionalLightCameraTransform;
    uint numberOfVolumetricStepsDir;
    float volumetricScatteringDir;
    float volumetricIntensityDir;
    float padding;
    float4 shadowColour;
    float4 ambianceColour;
};

cbuffer SpotlightData : register(b2)
{
    float3 lightPosition;
    float lightIntensity;
    float3 lightDirection;
    float lightRange;
    float3 lightColour;
    float lightInnerAngle;
    float lightOuterAngle;
    bool lightIsActive;
    bool useTexture;
    float4x4 spotLightCameraTransform;
    bool useVolumetricLight;
    uint numberOfVolumetricSteps;
    float volumetricScattering;
    float volumetricIntensity;
    float volumetricAngle;
    float volumetricRange;
    float volumetricFade;
};

// TODO Move shadowColour to common light or something
cbuffer RSMData : register(b3)
{
    bool isDirectional;
    uint sampleCount;
    float rMax;
    float rsmIntensity;
    float4x4 lightCameraTransform;
};

bool IsInSpotlightCone(float3 aWorldPosition, float aAngle)
{
    const float3 toLight = lightPosition - aWorldPosition;
    const float distToLight = length(toLight);
    const float3 lightDir = normalize(toLight);
    const float angle = dot(lightDir, lightDirection);

    return (angle > cos(aAngle)) && (distToLight < lightRange);
}

float4 main(DeferredVertexToPixel aInput) : SV_TARGET
{
    const float2 uv = aInput.position.xy / resolution.xy;
    const float3 worldPosition = gWorldPositionTex.Sample(linearSampler, uv).rgb;
    const float3 albedo = gColourTex.Sample(linearSampler, uv).rgb;
    const float3 normal = normalize(2.0f * gNormalTex.Sample(linearSampler, uv).xyz - 1.0f);
    const float4 material = gMaterialTex.Sample(linearSampler, uv);

    const float metalness = material.b;
    const float roughness = material.g;

    const float3 specular = lerp((float3) 0.04f, albedo.rgb, metalness);
    const float3 colour = lerp((float3) 0.0f, albedo.rgb, 1.0f - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - worldPosition);

    const float4 lightProjectedPositionTemp = mul(spotLightCameraTransform, float4(worldPosition, 1.0f));
    const float3 lightProjectedPosition = lightProjectedPositionTemp.xyz / lightProjectedPositionTemp.w;

    float shadowFactor = Shadow(lightProjectedPosition, spotLightShadowMap) + shadowColour.w;
    shadowFactor = saturate(shadowFactor);

    float3 lightFromTexture = float3(1.0f, 1.0f, 1.0f);

    if (useTexture)
    {
        float2 projectedPosition = lightProjectedPosition.xy;
        projectedPosition.x *= resolution.x / resolution.y; // Because texture has aspect ratio 1:1
        const float2 lightUV = 0.5f + float2(0.5f, -0.5f) * (projectedPosition.xy) / lightOuterAngle / PI; // 1.4835f is the spotlight camera fov (85) in radians

        lightFromTexture = flashlightTex.Sample(linearSampler, lightUV).rgb;
    }

    float3 volumetric = float3(0.0f, 0.0f, 0.0f);
    if (useVolumetricLight && lightIntensity > 100.0f)
    {
        float3 V = worldPosition - cameraPosition.xyz;

        const float stepSize = length(V) / (float) numberOfVolumetricSteps;
        V = normalize(V);
        const float3 step = V * stepSize;
 
        float3 position = cameraPosition.xyz;
        position += step * DITHER_PATTERN[int(uv.x * resolution.x) % 4][int(uv.y * resolution.y) % 4];
        
		[unroll(15)]
        for (int i = 0; i < numberOfVolumetricSteps; i++)
        {
            if (IsInSpotlightCone(position, volumetricAngle))
            {
                const float distToCamera = length(position - cameraPosition.xyz);

                float intensity = 0.0f;
                if (distToCamera < volumetricRange)
                {
                    const float fadeStart = volumetricRange - volumetricFade;

                    // Use smoothstep to interpolate between 1.0 and 0.0 based on the distance to the camera
                    intensity = smoothstep(volumetricRange, fadeStart, distToCamera);
                }
                volumetric += CalcScattering(dot(V, -lightDirection), volumetricScattering) * lightColour * intensity;
            }
            position += step;
        }
        volumetric /= (float) numberOfVolumetricSteps;
        volumetric *= volumetricIntensity;
    }

    float3 spotlight = EvaluateSpotLight(colour, specular, normal, roughness, lightFromTexture * lightColour, lightIntensity,
        lightRange, lightPosition, -lightDirection, lightOuterAngle, lightInnerAngle, toEye,
        worldPosition.xyz);

    if (shadowFactor < 1.0f)
    {
        spotlight *= shadowFactor;
    }

    return float4(spotlight.rgb + volumetric, 1.0f);
}
