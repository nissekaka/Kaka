cbuffer TransformCbuf : register(b0)
{
    matrix viewProj;
}

struct VertexInput
{
    float3 position : POSITION;
};

struct PixelInput
{
    float3 worldPosition : POSITION;
    float4 position : SV_POSITION;
};

PixelInput main(VertexInput aInput)
{
    PixelInput output;

    output.worldPosition = aInput.position;
    // 0.0f cancels out translation to only give rotation
    output.position = mul(float4(aInput.position, 0.0f), viewProj);
    // Make sure the depth after w divide will be 1.0
    output.position.z = output.position.w;
    
    return output;
}