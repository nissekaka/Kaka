float3 MapNormal(
    const in float3 aTan,
    const in float3 aBitan,
    const in float3 aNormal,
    const in float2 aTexCoord,
    uniform Texture2D aNmap,
    uniform SamplerState aSplr)
{
    // Build the tranform (rotation) into same space as tan/bitan/normal (target space)
    const float3x3 tanToTarget = float3x3(aTan, aBitan, aNormal);
    // Sample and unpack the normal from texture into target space   
    const float3 normalSample = aNmap.Sample(aSplr, aTexCoord).xyz;
    const float3 tanNormal = normalSample * 2.0f - 1.0f;
    // Bring normal from tanspace into target space
    return normalize(mul(tanNormal, tanToTarget));
}

float3 MapNormal(
    const in float3 aTan,
    const in float3 aBitan,
    const in float3 aNormal,
    const in float2 aTexCoord,
    const in float3 aNmap,
    uniform SamplerState aSplr)
{
    // Build the tranform (rotation) into same space as tan/bitan/normal (target space)
    const float3x3 tanToTarget = float3x3(aTan, aBitan, aNormal);
    // Sample and unpack the normal from texture into target space   
    const float3 normalSample = aNmap;
    const float3 tanNormal = normalSample * 2.0f - 1.0f;
    // Bring normal from tanspace into target space
    return normalize(mul(tanNormal, tanToTarget));
}

float Attenuate(uniform float aRadius, uniform float aFalloff, const in float aDistFragToL)
{
    float attenuation = clamp(1.0f - aDistFragToL / aRadius, 0.0f, 1.0f);
    attenuation = pow(attenuation, aFalloff);
    return attenuation;
}

float3 Diffuse(
    uniform float3 diffuseColour,
    uniform float diffuseIntensity,
    const in float att,
    const in float3 viewDirFragToL,
    const in float3 viewNormal)
{
    return diffuseColour * diffuseIntensity * att * max(0.0f, dot(viewDirFragToL, viewNormal));
}

float3 Speculate(
    const in float3 aSpecularColour,
    uniform float aSpecularIntensity,
    const in float3 aViewNormal,
    const in float3 aViewFragToL,
    const in float3 aViewPos,
    const in float aAtt,
    const in float aSpecularPower)
{
    // Calculate reflected light vector
    const float3 w = aViewNormal * dot(aViewFragToL, aViewNormal);
    const float3 r = normalize(w * 2.0f - aViewFragToL);
    // Vector from camera to fragment (in view space)
    const float3 viewCamToFrag = normalize(aViewPos);
    // Calculate specular component color based on angle between
    // viewing vector and reflection vector, narrow with power function
    return aAtt * aSpecularColour * aSpecularIntensity * pow(max(0.0f, dot(-r, viewCamToFrag)), aSpecularPower);
}