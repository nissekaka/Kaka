#include "common.hlsli"
#include "deferred_common.hlsli"
#include "PostprocessTonemap.hlsli"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D fullscreenTexture : register(t0);
Texture2D bloomSampleTexture : register(t1);

cbuffer Parameters : register(b1)
{
    float3 tint;
    float exposure;
    float3 blackpoint;
    float contrast;
    float saturation;
    float blur;
    float sharpness;
};

float4 main(const PixelInput aInput) : SV_TARGET
{
	//float4 colour = fullscreenTexture.Sample(linearSampler, aInput.texCoord).rgba;
    float4 colour = fullscreenTexture.Sample(linearSampler, aInput.texCoord);

	if (colour.a < 0.1f)
    {
        discard;
    }

    float4 bloom = bloomSampleTexture.Sample(linearSampler, aInput.texCoord).rgba;

    // Blur
    const float twoPi = 6.28318530718; // Pi*2
    
    const float directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    const float quality = 3.0; // BLUR QUALITY (Default 4.0 - More is better but slower)

    const float2 radius = blur / resolution.xy;
    
    // Blur calculations
    for (float d = 0.0; d < twoPi; d += twoPi / directions)
    {
        for (float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
        {
            colour += fullscreenTexture.Sample(linearSampler, aInput.texCoord + float2(cos(d), sin(d)) * radius * i);
            bloom += bloomSampleTexture.Sample(linearSampler, aInput.texCoord + float2(cos(d), sin(d)) * radius * i);
        }
    }
    
    // Output to screen
    colour /= quality * directions - 15.0f;
    bloom /= quality * directions - 15.0f;

    // Sharpening
    float3 sharpenedSum = colour.rgb * (1.0f + 4.0f * sharpness);

    sharpenedSum -= fullscreenTexture.Sample(linearSampler, aInput.texCoord + float2(0.0f, 1.0f) / resolution).rgb * sharpness;
    sharpenedSum -= fullscreenTexture.Sample(linearSampler, aInput.texCoord - float2(0.0f, 1.0f) / resolution).rgb * sharpness;
    sharpenedSum -= fullscreenTexture.Sample(linearSampler, aInput.texCoord + float2(1.0f, 0.0f) / resolution).rgb * sharpness;
    sharpenedSum -= fullscreenTexture.Sample(linearSampler, aInput.texCoord - float2(1.0f, 0.0f) / resolution).rgb * sharpness;

    colour = float4(sharpenedSum, 1.0f);

    float3 newColour = colour + bloom;

    // Saturation
    const float luminance = dot(float3(0.2126, 0.7152, 0.0722), colour.rgb);
    newColour = luminance + saturation * (newColour - luminance);

	// Exposure
    newColour = pow(2, exposure) * newColour;

    // Contrast
    newColour = 0.18f * pow(newColour / 0.18f, contrast); // 0.18 is perceived as 50% grey. Keep it constan

    // Blackpoint
    newColour = max(0.0f, newColour - blackpoint);

    // Tint
    newColour = newColour * tint;

    return float4(tonemap_s_gamut3_cine(newColour), 1.0f);
}