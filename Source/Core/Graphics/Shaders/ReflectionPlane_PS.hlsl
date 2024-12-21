#include "common.hlsli"
#include "deferred_common.hlsli"
#include "Light.hlsli"
#include "PBRFunctions.hlsli"

static const uint MAX_LIGHTS = 50u; // Needs to be the same in PointLight

TextureCube daySkyTex : register(t12);
TextureCube nightSkyTex : register(t13);

cbuffer Model : register(b0)
{
    bool normalMapEnabled;
    bool materialEnabled;
};

cbuffer DirectionalLight : register(b1)
{
    float3 dLightDirection;
    float padding;
    float3 dLightColour;
    float ambientLightPower;
    float4x4 directionalLightCameraTransform;
};

cbuffer PointLight : register(b2)
{
    PointlightData plBuf[MAX_LIGHTS];
    uint activePointLights;
}

cbuffer SpotLightBuffer : register(b3)
{
    SpotlightData slBuf[MAX_LIGHTS];
    uint activeSpotLights;
}

struct PixelInput
{
    float3 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
};

cbuffer Reflection : register(b10)
{
    const float2 k0 = { 0.0f, 0.0f };
    const float2 k1 = { 0.0f, 0.0f };
    const float A = 0.0f;
}

static const float WaveFrequency = 1.0f;
static const float WaveAmplitude = 0.05f;

Texture2D reflectTex : register(t2);
Texture2D colourTex : register(t3);
Texture2D normalTex : register(t4);
Texture2D materialTex : register(t5);

float4 main(PixelInput aInput) : SV_TARGET
{
    float waveOffset = sin(currentTime * WaveFrequency) * WaveAmplitude;
    const float2 texCoord = aInput.texCoord + float2(waveOffset, waveOffset);

    float3 colour = colourTex.Sample(defaultSampler, texCoord).rgb;
    float3 normal = normalTex.Sample(linearSampler, texCoord).xyz;
    const float3 material = materialTex.Sample(linearSampler, texCoord).rgb;

    float3 ambientLight = { 0.0f, 0.0f, 0.0f };
    float3 directionalLight = { 0.0f, 0.0f, 0.0f };
    float3 pointLight = { 0.0f, 0.0f, 0.0f };
    float3 spotLight = { 0.0f, 0.0f, 0.0f };
    float3 specular = { 0.0f, 0.0f, 0.0f };
    float ambientOcclusion = 0.0f;

    float metalness = 0.0f;
    float roughness = 0.0f;
    float emissive = 0.0f;
	
    if (normalMapEnabled)
    {
        normal = 2.0f * normal - 1.0f;
        normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y));
        normal = normalize(normal);

        float3x3 TBN = float3x3(
		normalize(aInput.tangent.xyz),
		normalize(-aInput.bitan.xyz),
		normalize(aInput.normal.xyz)
		);

	    // Can save an instruction here by instead doing
	    // normalize(mul(normal, TBN)); It works because
	    // TBN is a 3x3 and therefore TBN^T is the same
	    // as TBN^-1. However, it is considered good form to do this.
        TBN = transpose(TBN);
        normal = normalize(mul(TBN, normal));
    }

    if (materialEnabled)
    {
        ambientOcclusion = material.r;
        roughness = 1.0f; //material.g;
        metalness = 0.0f; //material.b;
        //emissive = material.b;

        specular = lerp((float3) 0.64f, colour.rgb, metalness);
        colour = lerp((float3) 0.0f, colour.rgb, 1 - metalness);
    }

    const float3 toEye = normalize(cameraPosition.xyz - aInput.worldPos.xyz);

    // Shadows

    //const float shadowFactor = Shadow(directionalLightCameraTransform, float4(aInput.worldPos, 1.0f), directionalLightShadowMap);

    // Lighting

	// Day/night cycle
    const float3 lightDir = normalize(dLightDirection);
    const float dotProduct = dot(lightDir, float3(0.0f, 1.0f, 0.0f));
    const float blendFactor = (dotProduct + 1.0) / 2.0f;

    // Ambient light
    ambientLight = EvaluateAmbianceDynamicSky(defaultSampler, daySkyTex, nightSkyTex, blendFactor,
    normal, aInput.normal.xyz, toEye, roughness, ambientOcclusion, colour, specular);

	// Directional light
    directionalLight = EvaluateDirectionalLight(colour, specular, normal,
    roughness, dLightColour, -dLightDirection, toEye);

    // Point lights
    for (uint i = 0; i < activePointLights; ++i)
    {
        if (!plBuf[i].active)
        {
            continue;
        }

        pointLight += EvaluatePointLight(colour, specular, normal,
        roughness, plBuf[i].colour, plBuf[i].intensity,
        plBuf[i].radius, plBuf[i].position, toEye, aInput.worldPos);
    }

	// Spot lights
    for (uint i = 0; i < activeSpotLights; ++i)
    {
        if (!slBuf[i].active)
        {
            continue;
        }

        spotLight += EvaluateSpotLight(colour, specular, normal, roughness, slBuf[i].colour, slBuf[i].intensity,
        slBuf[i].range, slBuf[i].position, slBuf[i].direction, slBuf[i].outerAngle, slBuf[i].innerAngle, toEye, aInput.worldPos);
    }

	// Final colour
    const float3 finalColour = saturate(ambientLight * ambientLightPower + directionalLight + pointLight + spotLight);
    
    // Reflection
    const float dist = abs(dot(toEye, cameraPosition.xyz));
    const float2 p = aInput.worldPos.xz;
	// Derivative with respect to x and z of height sin(dot(p, k0) + time) + sin(dot(p, k1) + time)
    const float2 heightDerivative = k0 * sin(dot(p, k0) + currentTime) + k1 * sin(dot(p, k1) + currentTime * 5.0f);
    const float2 maxValue = float2(0.f, length(k0) + length(k1));
    // Adding max values of heightDerivative to not sample to high
    // Dividing by dist to make waves smaller at distance
    // Scaling by Amplitude parameter
    const float2 offset = A * (maxValue + heightDerivative) / dist;
    const float3 reflection = reflectTex.Sample(defaultSampler, aInput.position.xy / resolution + offset).rgb;

    const float3 result = lerp(reflection, finalColour /*shadowFactor*/, 0.35f);

    return float4(result, 0.5f);
}