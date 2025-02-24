
#include "../Common/deferred_common.hlsli"

struct VertexInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
    uint instanceID : SV_InstanceID;
};

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

StructuredBuffer<matrix> InstanceTransforms : register(t11);

PixelInput main(const VertexInput aInput)
{
    PixelInput output;

	const matrix objectToWorld = InstanceTransforms[aInput.instanceID];
    const float3x3 objectToWorldRotation = objectToWorld;
    const float4 position = { aInput.position, 1.0f };
    output.worldPos = mul(objectToWorld, position).xyz;
    output.position = mul(viewProjection, float4(output.worldPos, 1.0f));
    output.previousPosition = mul(historyViewProjection, float4(output.worldPos, 1.0f));
    output.texCoord = aInput.texCoord;
    output.normal = mul(objectToWorldRotation, aInput.normal);
    output.worldNormal = aInput.normal;
    output.tangent = mul(objectToWorldRotation, aInput.tan);
    output.bitan = mul(objectToWorldRotation, aInput.bitan);
    
    return output;
}

