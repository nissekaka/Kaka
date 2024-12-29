struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PixelInput main(const VertexInput aInput)
{
    PixelInput output;

    output.position = aInput.position;    
    output.texCoord = aInput.texCoord;
    
    return output;
}