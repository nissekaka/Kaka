

Texture2D vfxTexture : register(t7);
SamplerState vfxSampler : register(s7);

struct VFXVertexToPixelInput
{
    float3 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct VFXOutput
{
    float4 colour : SV_TARGET;
};