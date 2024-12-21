#include "deferred_common.hlsli"
#include "Poisson.hlsli"
#include "common.hlsli"

Texture2D rsmWorldPositionTex : register(t6);
Texture2D rsmNormalTex : register(t7);
Texture2D rsmFluxTex : register(t8);

cbuffer RSMData : register(b3)
{
    bool isDirectional;
    uint sampleCount;
    float rMax;
    float rsmIntensity;
    float4x4 lightCameraTransform;
};

#define PI (3.141592653)
#define TWO_PI (2.0 * PI)

// Reverses the bits of the input
uint BitfieldReverse(const uint aBits)
{
    uint b = (uint(aBits) << 16u) | (uint(aBits) >> 16u);
    b = (b & 0x55555555u) << 1u | (b & 0xAAAAAAAAu) >> 1u;
    b = (b & 0x33333333u) << 2u | (b & 0xCCCCCCCCu) >> 2u;
    b = (b & 0x0F0F0F0Fu) << 4u | (b & 0xF0F0F0F0u) >> 4u;
    b = (b & 0x00FF00FFu) << 8u | (b & 0xFF00FF00u) >> 8u;
    return b;
}

float2 Hammersley(const uint aI, const uint aN)
{
    return float2(float(aI) / float(aN), float(BitfieldReverse(aI)) * 2.3283064365386963e-10);
}

float2 hash23(float3 p3)
{
    p3 = frac(p3 * float3(0.1031f, 0.1030f, 0.0973f));
    p3 += dot(p3, p3.yzx + 33.33f);
    return frac((p3.xx + p3.yz) * p3.zy);
}


float3 IndirectLighting(const float2 aUv, const float3 aN, const float3 aX, const float aRMax, const uint aSampleCount, const float aIntensity, const float2 aPixel)
{
    // The irradiance at a surface point x with normal n due to pixel light p is
    //
    //                     max{0,<np|aX-xp>} max{0,<aN| xp-aX>}
    // Ep(aX, aN) = phi(p) -----------------------------------             
	//                                ||aX-xp||^4

    float3 output = { 0.0f, 0.0f, 0.0f };
    const float2 noise = hash23(float3(aPixel.x, aPixel.y, currentTime * 1000.0f));

	[loop]
    for (uint i = 0; i < aSampleCount; i++)	// Sum contributions of sampling locations
    {
        float2 offset = Hammersley(i, aSampleCount);

        offset.xy += noise;
        offset.xy = fmod(offset.xy, 1.0f);
            
    	// Soft radius rather than hard cutoff
        offset.x = (offset.x * offset.x * offset.x * offset.x + 1.0f) * offset.x;

        float r = offset.x * aRMax;
        float theta = offset.y * TWO_PI;
        float2 coord = aUv + float2(r * cos(theta), r * sin(theta));
        float weight = offset.x * offset.x;

        float3 xp = rsmWorldPositionTex.Sample(linearSampler, coord).xyz; // Position (x_p) and normal (n_p) are in world coordinates too
        float3 flux = rsmFluxTex.Sample(linearSampler, coord).rgb; // Collect components from corresponding RSM textures
        float3 np = normalize(2.0f * rsmNormalTex.Sample(linearSampler, coord).xyz - 1.0f);

        float3 Ep = flux * ((max(0, dot(np, aX - xp)) * max(0, dot(aN, xp - aX)))
									/ pow(length(aX - xp), 4));

        Ep *= weight; // Weighting contribution and normalizing

        output += Ep; // Accumulate
    }

    return output / aSampleCount * aIntensity; // Modulate result with some intensity value
}