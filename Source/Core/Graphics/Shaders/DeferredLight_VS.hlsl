#include "deferred_common.hlsli"

cbuffer DeferredVertexConstantBuffer : register(b1)
{
    float4 lightPosAndRange;
    bool isDirectional;
};

DeferredVertexToPixel main(DeferredVertexInput aInput)
{
    DeferredVertexToPixel output;
    
    output.position = float4(aInput.position, 1.0f);

    if (!isDirectional)
    {
        const float4 position = { aInput.position, 1.0f };

        matrix objectToWorld = matrix(
        lightPosAndRange.w, 0, 0, lightPosAndRange.x,
        0, lightPosAndRange.w, 0, lightPosAndRange.y,
        0, 0, lightPosAndRange.w, lightPosAndRange.z,
		0, 0, 0, 1
        );

        output.position = mul(objectToWorld, position);
        output.position = mul(viewProjection, output.position);
    }

    return output;
}