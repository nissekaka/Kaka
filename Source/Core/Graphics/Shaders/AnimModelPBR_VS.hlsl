cbuffer Transform : register(b0)
{
    matrix objectToWorld;
    matrix objectToClip;
}

cbuffer SkeletonBuffer : register(b1)
{
    float4x4 boneTransforms[64u]; // Array of bone transforms
};

float3x3 invertMatrix(float3x3 m)
{
	// computes the inverse of a matrix m
    float det = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
		m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
		m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    float invdet = 1 / det;

    float3x3 minv; // inverse of matrix m
    minv[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invdet;
    minv[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invdet;
    minv[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invdet;
    minv[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invdet;
    minv[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invdet;
    minv[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invdet;
    minv[2][0] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invdet;
    minv[2][1] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invdet;
    minv[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invdet;

    return minv;
}

struct VertexInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
    uint4 boneIndices : BONEINDICES;
    float4 boneWeights : BONEWEIGHT;
};

struct PixelInput
{
    float3 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
};

PixelInput main(const VertexInput aInput)
{
    PixelInput output;

	// Accumulate bone transformations based on bone indices and weights
    matrix finalTransform =
    {
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 }
    };

    for (int i = 0; i < 4; ++i)
    {
        const uint boneIndex = aInput.boneIndices[i];
        const float boneWeight = aInput.boneWeights[i];

		// Fetch the bone transform from the skeleton buffer
        const matrix boneTransform = boneTransforms[boneIndex];

        // Accumulate the transformed contribution from each bone
        finalTransform += boneWeight * boneTransform;
    }

	// Apply the final bone transform to the vertex position
    const float4 worldPosition = mul(finalTransform, float4(aInput.position, 1.0f));

    const float3x3 objectToWorldRotation = objectToWorld;
    const float3x3 skinnedRotation = finalTransform;

    const float3 vertexWorldNormal = mul(transpose(invertMatrix(objectToWorldRotation)), mul(transpose(invertMatrix(skinnedRotation)), aInput.normal));
    const float3 vertexWorldTangent = mul(objectToWorldRotation, mul(skinnedRotation, aInput.tan));
    const float3 vertexWorldBinormal = mul(objectToWorldRotation, mul(skinnedRotation, aInput.bitan));

    const float4 position = worldPosition;
    output.worldPos = mul(objectToWorld, position).xyz;
    output.position = mul(objectToClip, position);
    output.texCoord = aInput.texCoord;
    output.normal = vertexWorldNormal;
    output.tangent = vertexWorldTangent;
    output.bitan = vertexWorldBinormal;
    
    return output;
}