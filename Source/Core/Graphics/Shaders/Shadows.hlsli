
//#include "PCSS.hlsli"
#include "Poisson.hlsli"
#include "common.hlsli"

Texture2D directionalLightShadowMap : register(t14);
Texture2D spotLightShadowMap : register(t15);

cbuffer ShadowBuffer : register(b7)
{
    bool usePCF;
    float offsetScalePCF;
    int sampleCountPCF;
    bool usePoisson;
    float offsetScalePoissonDisk;
}

float PoissonDisk(float3 aLightProjectedPosition, Texture2D aTexture)
{
    const float computedZ = aLightProjectedPosition.z;
    const float bias = 0.001f;

    float shadowFactor = 0.0f;

    const int sampleCount = 64;
    const float offsetScale = offsetScalePoissonDisk;

    [unroll(64)] // Unroll the loop (64 samples in the Poisson disk pattern)]
    for (int i = 0; i < sampleCount; ++i)
    {
        const float2 sampleOffset = POISSON_DISK_64[i];

		// Adjust sampleUV based on facing direction
        const float2 sampleUV = 0.5f + float2(0.5f, -0.5f) * (aLightProjectedPosition.xy) + sampleOffset * offsetScale;

        const float shadowMapZ = aTexture.Sample(linearSampler, sampleUV);

        //shadowFactor -= (1.0f / 16.0f) * clamp(1.0f - shadowMapZ + bias, 0.0f, 1.0f);
        shadowFactor += (computedZ < shadowMapZ + bias) ? 1.0f : 0.0f;
    }

    return shadowFactor / (float)sampleCount;
}

float PCF(float3 aLightProjectedPosition, Texture2D aTexture)
{
    const float computedZ = aLightProjectedPosition.z;
    const float bias = 0.001f;
    
    float shadowFactor = 0.0f;

	// Filter kernel for PCF eg. (15x15)
    const int sampleCount = clamp(sampleCountPCF, 3, 25);
	// Offset scale decides how much the shadow edge is moved for "blurring"
    const float offsetScale = offsetScalePCF;

    [unroll(25)] // Unroll the loop (max 25x25 samples in the PCF pattern)]
    for (int i = -sampleCount / 2u; i <= sampleCount / 2u; ++i)
    {
        [unroll(25)]
        for (int j = -sampleCount / 2u; j <= sampleCount / 2u; ++j)
        {
            const float2 sampleOffset = float2(i, j) / float(sampleCount);
            const float2 sampleUV = 0.5f + float2(0.5f, -0.5f) * (aLightProjectedPosition.xy + sampleOffset * offsetScale);
            const float shadowMapZ = aTexture.Sample(linearSampler, sampleUV);

            shadowFactor += (computedZ < shadowMapZ + bias) ? 1.0f : 0.0f;
        }
    }

	// Average the results
    return shadowFactor / float(sampleCount * sampleCount);
}

float Shadow(const in float3 aProjectedPosition, Texture2D aTexture)
{
    float shadowFactor = 1.0f;
    if (clamp(aProjectedPosition.x, -1.0f, 1.0f) == aProjectedPosition.x &&
        clamp(aProjectedPosition.y, -1.0f, 1.0f) == aProjectedPosition.y)
    {
        float shadowFactorPCF = 0.0f;
        float shadowFactorPoisson = 0.0f;

        if (usePoisson)
        {
            shadowFactorPoisson = PoissonDisk(aProjectedPosition, aTexture);
            shadowFactor = shadowFactorPoisson;
        }
        if (usePCF)
        {
            shadowFactorPCF = PCF(aProjectedPosition, aTexture);
            shadowFactor = shadowFactorPCF;
        }
        if (usePoisson && usePCF)
        {
            shadowFactor = (shadowFactorPoisson + shadowFactorPCF) / 2.0f;
        }
    }

    return shadowFactor;
}