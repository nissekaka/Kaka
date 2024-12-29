static const float4x4 DITHER_PATTERN = float4x4
    (float4(0.0f, 0.5f, 0.125f, 0.625f),
     float4(0.75f, 0.22f, 0.875f, 0.375f),
     float4(0.1875f, 0.6875f, 0.0625f, 0.5625f),
     float4(0.9375f, 0.4375f, 0.8125f, 0.3125f));

#define PI (3.141592653)

// Mie scaterring approximated with Henyey-Greenstein phase function.
float CalcScattering(float aCosTheta, float aScattering)
{
    return (1.0f - aScattering * aScattering) / (4.0f * PI * pow(1.0f + aScattering * aScattering - 2.0f * aScattering * aCosTheta, 1.5f));
}

float3 Volumetric(float3 aWorldPosition, float3 aCameraPosition, uint aSteps, float2 aUv, float2 aResolution, matrix aTransform, Texture2D aShadowMap, SamplerState aSampler, float3 aLightDirection, float3 aColour, float aScattering)
{
    float3 volumetric = float3(0.0f, 0.0f, 0.0f);
    float3 V = aWorldPosition - aCameraPosition;

    const float stepSize = length(V) / (float)aSteps;
    V = normalize(V);
    const float3 step = V * stepSize;
 
    float3 position = aCameraPosition;
    position += step * DITHER_PATTERN[int(aUv.x * aResolution.x) % 4][int(aUv.y * aResolution.y) % 4];

    [unroll(30)]
    for (int i = 0; i < aSteps; i++)
    {
        const float4 lightSpacePositionTemp = mul(aTransform, float4(position, 1.0f));
        const float3 lightSpacePosition = lightSpacePositionTemp.xyz / lightSpacePositionTemp.w;

        const float2 sampleUV = 0.5f + float2(0.5f, -0.5f) * lightSpacePosition.xy;

        const float shadowMapValue = aShadowMap.Sample(aSampler, sampleUV).r;

        if (shadowMapValue > lightSpacePosition.z)
        {
            volumetric += CalcScattering(dot(V, -aLightDirection), aScattering) * aColour;
        }
        position += step;
    }
    volumetric /= (float) aSteps;

    return volumetric;
}