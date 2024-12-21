#include "common.hlsli"
#include "deferred_common.hlsli"

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

cbuffer TAABuffer : register(b1)
{
    float2 jitterOffset;
    float2 previousJitterOffset;
    bool useTAA;
};

Texture2D colourTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D materialTex : register(t3);

GBufferOutput main(PixelInput aInput)
{
    float4 albedo = colourTex.Sample(defaultSampler, aInput.texCoord).rgba;

    if (albedo.a < 0.5f)
    {
        discard;
    }

    float3 normal = float3(normalTex.Sample(linearSampler, aInput.texCoord).rg, 1.0f);
    //float3 normal = normalTex.Sample(defaultSampler, scaledUV).wyz;
    //float ambientOcclusion = normal.z;
    normal = 2.0f * normal - 1.0f;

    // normal.z calculation is
    // not needed for Sponza
    // but is needed models with
    // only x and y normal data
    normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y)); 
    normal = normalize(normal);
    normal.y = -normal.y;

    float3x3 TBN = float3x3(normalize(aInput.tangent.xyz),
                            normalize(-aInput.bitan.xyz),
                            normalize(aInput.normal.xyz));
    TBN = transpose(TBN);

    const float3 pixelNormal = normalize(mul(TBN, normal));

    float4 material = materialTex.Sample(linearSampler, aInput.texCoord);
    float ambientOcclusion = material.r;

    GBufferOutput output;
    output.worldPosition = float4(aInput.worldPos, 1.0f);
    output.albedo = float4(albedo.rgb, 1.0f);
    output.normal = float4(0.5f + 0.5f * pixelNormal, 1.0f);
    output.material = float4(material.rgb, 1.0f);
    output.ambientOcclusionAndCustom = float4(ambientOcclusion, aInput.worldNormal); // gba are unused, put whatever data you want here!
    output.ambientOcclusionAndCustom.g = 0.0f;

    const float4 location_ndc = aInput.position / aInput.position.w;
    const float4 previous_location_ndc = aInput.position / aInput.position.w;

    const float2 previous_uv = 0.5 + previous_location_ndc.xy * float2(0.5, -0.5);
    const float2 previous_pixel_location = previous_uv * resolution;

    const float2 pixel_motion_vectors = previous_pixel_location - previousJitterOffset - location_ndc - jitterOffset;

    output.velocity = pixel_motion_vectors;
    //const float2 previousPosNDC = aInput.previousPosition.xy / aInput.previousPosition.w;
    //const float2 currentPosNDC = aInput.position.xy / aInput.position.w;

    //const float2 velocity = previousPosNDC - currentPosNDC;
    //output.velocity = velocity * resolution;// * float2(0.5, -0.5); // Put in UV space
    //output.velocity -= jitterOffset;
    //output.velocity -= previousJitterOffset;

    return output;
}

