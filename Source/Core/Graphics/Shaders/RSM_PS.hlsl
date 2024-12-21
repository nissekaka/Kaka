#include "common.hlsli"
#include "deferred_common.hlsli"
//#include "GBuffer.hlsli"

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

cbuffer RSMLightData : register(b0)
{
    float4 lightColourAndIntensity;
    float4 lightDirectionAndInnerAngle;
    float4 lightPositionAndOuterAngle;
    float range;
    bool isDirectionalLight;
}

Texture2D colourTex : register(t1);
Texture2D normalTex : register(t2);

RSMBufferOutput main(PixelInput aInput)
{
    //const float2 uv = aInput.position.xy / clientResolution.xy;
    float4 albedo = colourTex.Sample(defaultSampler, aInput.texCoord).rgba;
    if (albedo.a < 0.1f)
    {
        discard;
    }

    //const float3 normal = normalize(2.0f * gNormalTex.Sample(linearSampler, aInput.texCoord).xyz - 1.0f);
    //float3 normal = normalize(2.0f * gNormalTex.Sample(linearSampler, aInput.texCoord).xyz - 1.0f);
    //float3 worldNormal = normalize(2.0f * normalTex.Sample(linearSampler, aInput.texCoord).xyz - 1.0f);

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

    //float ambientOcclusion = normal.z;
    //normal = 2.0f * normal - 1.0f;
	// normal.z calculation is
    // not needed for Sponza
    // but is needed models with
    // only x and y normal data
    //normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y));
    //normal = normalize(normal);
    //normal.y = -normal.y;

    //float3x3 TBN = float3x3(normalize(aInput.tangent.xyz),
    //                        normalize(-aInput.bitan.xyz),
    //                        normalize(aInput.normal.xyz));
    //TBN = transpose(TBN);
    //
    //const float3 pixelNormal = normalize(mul(TBN, normal));

    RSMBufferOutput output;

    output.worldPosition = float4(aInput.worldPos, 1.0f);
    if (isDirectionalLight)
    {
        output.flux = float4(albedo.rgb * lightColourAndIntensity.xyz, 1.0f) * lightColourAndIntensity.w;
    }
    else
    {
        float3 toLight = lightPositionAndOuterAngle.xyz - aInput.worldPos.xyz;
        const float lightDistance = length(toLight);
        toLight /= lightDistance; // Normalize in-place

        const float cosOuterAngle = cos(lightPositionAndOuterAngle.w);
        const float cosInnerAngle = cos(lightDirectionAndInnerAngle.w);
        const float3 lightDirection = -lightDirectionAndInnerAngle.xyz;

		// Simplify cone check
		const float theta = dot(toLight, -lightDirection);
        float intensity = saturate((theta - cosOuterAngle) / (cosInnerAngle - cosOuterAngle));
        intensity *= intensity;

		// Simplify ue4Attenuation without using pow
        const float attenuationFactor = saturate(1 - lightDistance / range);
        const float ue4Attenuation = attenuationFactor * attenuationFactor / (lightDistance * lightDistance + 1);

		// Combine terms for final attenuation
        const float finalAttenuation = saturate(theta) * intensity * ue4Attenuation;


        output.flux = float4(albedo.rgb * lightColourAndIntensity.rgb, 1.0f) * finalAttenuation * lightColourAndIntensity.w;
    }

    output.normal = float4(0.5f + 0.5f * pixelNormal, 1.0f);;

    return output;
}