struct VertexInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    uint instanceID : SV_InstanceID;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

StructuredBuffer<matrix> InstanceTransforms : register(t11);

PixelInput main(const VertexInput aInput)
{
    PixelInput output;
    
    const float4 position = { aInput.position, 1.0f };
    output.position = mul(InstanceTransforms[aInput.instanceID], position);
    output.texCoord = aInput.texCoord;
    
    return output;
}