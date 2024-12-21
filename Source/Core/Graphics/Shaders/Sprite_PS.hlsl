#include "Shadows.hlsli"

struct PixelInput
{
    float3 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldNormal : WORLDNORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 colour : INSTANCE_COLOUR;
};

Texture2D gColourTex : register(t1);

float4 main(PixelInput aInput) : SV_TARGET
{
    float4 colour = gColourTex.Sample(linearSampler, aInput.texCoord).rgba;
    colour.rgb *= aInput.colour.rgb;
    
    // TODO This should be set on the CPU side
    float spotlightAlpha = 0.15f;

    colour.a *= spotlightAlpha * aInput.colour.a;

    return float4(colour);
}
