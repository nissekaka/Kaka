#include "Shadows.hlsli"

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

Texture2D gColourTex : register(t1);

cbuffer FlashlightData : register(b2)
{
    float3 lightPosition;
    float volumetricRange;
    float3 lightDirection;
    float angleInner;
    float angleOuter;
    float intensityInner;
    float intensityOuter;
    float alphaInner;
    float alphaOuter;
};

bool IsInSpotlightCone(float3 aWorldPosition, float aAngle)
{
    const float3 toLight = lightPosition - aWorldPosition;
    const float distToLight = length(toLight);
    const float3 lightDir = normalize(toLight);
    const float angle = dot(lightDir, lightDirection);

    return (angle > cos(aAngle)) && (distToLight < volumetricRange);
}

float4 main(PixelInput aInput) : SV_TARGET
{
    float4 colour = gColourTex.Sample(linearSampler, aInput.texCoord).rgba;
    colour.rgb *= aInput.colour.rgb;

    const float distToLight = length(lightPosition - aInput.worldPos);
    
    float spotlightAlpha = 0.15f;
    if (IsInSpotlightCone(aInput.worldPos, angleInner) && intensityInner > 200.0f)
    {
        if (distToLight < 100.0f)
        {
            spotlightAlpha = smoothstep(100.0f, 0.0f, distToLight) * alphaInner;
        }
        else
        {
            spotlightAlpha = 0.15f;
        }
    }
    else if (IsInSpotlightCone(aInput.worldPos, angleOuter) && intensityOuter > 200.0f)
    {
        if (distToLight < 100.0f)
        {
            spotlightAlpha = smoothstep(100.0f, 0.0f, distToLight) * alphaOuter;
        }
        else
        {
            spotlightAlpha = 0.15f;
        }
    }

    colour.a *= spotlightAlpha * aInput.colour.a;

    return float4(colour);
}
