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

float3 ReconstructWorldPosition(float2 uv, float depth01)
{
    // UV [0,1] -> NDC [-1,1]
    float4 ndc;
    ndc.x = uv.x * 2.0f - 1.0f;
    ndc.y = 1.0f - uv.y * 2.0f; // Y flip
    ndc.z = depth01; // D3D depth in [0,1]
    ndc.w = 1.0f;

    // Clip -> World
    float4 world = mul(inverseViewProjection, ndc);
    world /= world.w;
    return world.xyz;
}

struct GBufferOutput
{
    //float4 worldPosition : SV_TARGET0;
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 material : SV_TARGET2;
    float4 ambientOcclusionAndCustom : SV_TARGET3;
    //float2 velocity : SV_TARGET5;
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