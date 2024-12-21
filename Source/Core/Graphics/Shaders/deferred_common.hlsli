cbuffer CommonBuffer : register(b4)
{
			matrix viewProjection;
			matrix historyViewProjection;
			matrix inverseViewProjection;
			matrix projection;
			matrix viewInverse;
			float4 cameraPosition;
			float2 resolution;
			float currentTime;
}

struct GBufferOutput
{
    float4 worldPosition : SV_TARGET0;
    float4 albedo : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float4 material : SV_TARGET3;
    float4 ambientOcclusionAndCustom : SV_TARGET4;
    float2 velocity : SV_TARGET5;
    //float4 rsm : SV_TARGET5;
};

struct RSMBufferOutput
{
    float4 worldPosition : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 flux : SV_TARGET2;
};
struct DeferredVertexInput
{
    float3 position : POSITION;
};

struct DeferredVertexToPixel
{
    float4 position : SV_POSITION;
};