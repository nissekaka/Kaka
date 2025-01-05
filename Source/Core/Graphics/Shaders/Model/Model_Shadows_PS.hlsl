#include "../Common/common.hlsli"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
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

