#include "../Common/common.hlsli"
#include "../Common/deferred_common.hlsli"

struct PixelInput
{
    float3 worldPos : POSITION;
    float4 position : SV_POSITION;
    float4 previousPosition : PREVIOUS_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldNormal : WORLDNORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
};

Texture2D colourTex : register(t1);

void main(PixelInput aInput)
{
    float4 albedo = colourTex.Sample(pointSampler, aInput.texCoord).rgba;

    if (albedo.a < 0.5f)
    {
        discard;
    }

    return;
}

