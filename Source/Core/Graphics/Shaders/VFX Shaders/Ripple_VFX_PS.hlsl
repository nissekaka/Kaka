#include "VFX_Commons.hlsli"
#include "../deferred_common.hlsli"
#include "../common.hlsli"

Texture2D worldPosTex : register(t12);

VFXOutput main(VFXVertexToPixelInput aInput)
{
    VFXOutput output;

    const float2 uv = aInput.position.xy / resolution.xy;
    const float3 worldPosition = worldPosTex.Sample(linearSampler, uv).rgb;

    float dist = length(worldPosition - aInput.worldPos);

    float2 texcoord = aInput.uv;
    texcoord.x += cos(currentTime);
    texcoord.y += sin(texcoord.x);

    float alpha = smoothstep(200.0, 300.0f, dist);
    alpha *= 0.5f + 0.5f * sin(currentTime * texcoord.x);

    const float3 blueIsh = float3(0.0f, 0.5f, 1.0f);

    output.colour = vfxTexture.Sample(vfxSampler, texcoord);
    output.colour = float4(output.colour.rgb * blueIsh, alpha);

    return output;
}