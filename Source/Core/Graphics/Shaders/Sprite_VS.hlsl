#include "deferred_common.hlsli"

cbuffer Transform : register(b0)
{
    matrix objectToWorld; // Uneccessary
    matrix objectToClip;
}

struct VertexInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
    uint instanceID : SV_InstanceID;
};

struct InstanceData
{
    matrix instanceTransform;
    float4 colour;
};

StructuredBuffer<InstanceData> instanceDataBuffer : register(t11);

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

PixelInput main(const VertexInput aInput)
{
    PixelInput output;
    
    const matrix instObjectToWorld = instanceDataBuffer[aInput.instanceID].instanceTransform;
    const matrix instObjectToClip = mul(viewProjection, instObjectToWorld);

    const float3x3 objectToWorldRotation = objectToWorld;
    const float4 position = aInput.position;
    output.worldPos = mul(instObjectToWorld, position).xyz;
    output.position = mul(instObjectToClip, position);
    output.texCoord = aInput.texCoord;
    output.normal = mul(objectToWorldRotation, aInput.normal);
    output.tangent = mul(objectToWorldRotation, aInput.tan);
    output.bitan = mul(objectToWorldRotation, aInput.bitan);
    output.colour = instanceDataBuffer[aInput.instanceID].colour;
    output.worldNormal = aInput.normal;
    
    return output;
}