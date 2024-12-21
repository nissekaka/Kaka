#include "common.hlsli"
#include "deferred_common.hlsli"
#include "GBuffer.hlsli"

struct PixelInput
{
    float3 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldNormal : WORLDNORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
};

//Texture2D colourTex : register(t11);

GBufferOutput main(PixelInput aInput) : SV_TARGET
{
    float4 albedo = gColourTex.Sample(defaultSampler, aInput.texCoord).rgba;

    if (albedo.a < 0.1f)
    {
        discard;
    }

   // const float2 uv = aInput.position.xy / clientResolution.xy;
    //const float3 worldPosition = gWorldPositionTex.Sample(defaultSampler, uv).rgb;

    //const float distToWorld = length(worldPosition - aInput.worldPos);
    //const float distToCamera = length(cameraPosition.xyz - aInput.worldPos);

    // Smoothstep the alpha
    //const float worldAlpha = smoothstep(0.0, 15.0f, distToWorld); // * 0.05f;
    //const float cameraAlpha = smoothstep(0.0, 15.0f, distToCamera); // * 0.05f;

    //albedo.a = min(worldAlpha, cameraAlpha);
    //float3 normal = aInput.normal;
    float3 normal = float3(gNormalTex.Sample(linearSampler, aInput.texCoord).rg, 1.0f);
    //float3 normal = normalTex.Sample(defaultSampler, scaledUV).wyz;
    //float ambientOcclusion = normal.z;
    normal = 2.0f * normal - 1.0f;
    normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y));
    normal = normalize(normal);
    normal.y = -normal.y;

    float3x3 TBN = float3x3(normalize(aInput.tangent.xyz),
                            normalize(-aInput.bitan.xyz),
                            normalize(aInput.normal.xyz));
    TBN = transpose(TBN);

    const float3 pixelNormal = normalize(mul(TBN, normal));

    GBufferOutput output;
    output.worldPosition = float4(aInput.worldPos, 1.0f);
    output.albedo = float4(albedo.rgb, 1.0f);
    //output.normal = float4(aInput.normal, 1.0f);
    output.normal = float4(0.5f + 0.5f * pixelNormal, 1.0f);
    output.material = float4(0.5f, 0.5f, 0.5f, 1.0f);
    output.ambientOcclusionAndCustom = float4(0.0f, aInput.worldNormal); // gba are unused, put whatever data you want here!
    output.ambientOcclusionAndCustom.g = 0.0f;
    
    return output;
}