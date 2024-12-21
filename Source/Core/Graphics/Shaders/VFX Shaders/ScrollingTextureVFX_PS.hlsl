#include "VFX_Commons.hlsli"
#include "../deferred_common.hlsli"


VFXOutput main(VFXVertexToPixelInput aInput)
{
    VFXOutput output;

    float2 texcoord = aInput.uv;

    //texcoord.x *= currentTime;

    texcoord.y *= sin(currentTime * texcoord.x);

    output.colour = vfxTexture.Sample(vfxSampler, texcoord);

	return output;
}