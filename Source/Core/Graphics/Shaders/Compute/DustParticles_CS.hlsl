#define PI (3.141592653)

cbuffer ParticleConstants : register(b0)
{
    float4 cameraForward;
    float4 cameraRight;
    float4 cameraUp;
    float3 cameraPosition;
    float deltaTime;
    float elapsedTime;
    float maxRange;
};

struct InstanceData
{
    matrix instanceTransform;
    float4 colour;
};

struct ParticleData
{
    float3 startPosition;
    float travelRadius;
    float3 travelAngle;
    float fadeSpeed;
    float4 colour;
    float3 travelSpeed;
    float padding;
};

RWStructuredBuffer<InstanceData> instanceData : register(u0);
RWStructuredBuffer<ParticleData> particleData : register(u1);

// Main compute shader function
[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    //[unroll(2)]
    //for (int i = 0; i < 2; ++i)
    //{
    const uint updateIndex = DTid.x;

	// Update particle properties
    particleData[updateIndex].travelAngle.x -= particleData[updateIndex].travelSpeed.x * deltaTime;
    particleData[updateIndex].travelAngle.y -= particleData[updateIndex].travelSpeed.y * deltaTime;
    particleData[updateIndex].travelAngle.z -= particleData[updateIndex].travelSpeed.z * deltaTime;

    matrix transform = transpose(instanceData[updateIndex].instanceTransform);

    transform[3][0] = particleData[updateIndex].travelRadius * cos(particleData[updateIndex].travelAngle.x) + particleData[updateIndex].startPosition.x;
    transform[3][1] = particleData[updateIndex].travelRadius * sin(particleData[updateIndex].travelAngle.y) + particleData[updateIndex].startPosition.y;
    transform[3][2] = particleData[updateIndex].travelRadius * sin(particleData[updateIndex].travelAngle.z) + particleData[updateIndex].startPosition.z;
    transform[0] = cameraRight;
    transform[1] = cameraForward;
    transform[2] = cameraUp;

    instanceData[updateIndex].instanceTransform = transpose(transform);

    if (transform[3][0] < cameraPosition.x - maxRange)
    {
        particleData[updateIndex].startPosition.x = transform[3][0] + maxRange * 2.0f;
    }
    else if (transform[3][0] > cameraPosition.x + maxRange)
    {
        particleData[updateIndex].startPosition.x = transform[3][0] - maxRange * 2.0f;
    }

    if (transform[3][1] < cameraPosition.y - maxRange)
    {
        particleData[updateIndex].startPosition.y = transform[3][1] + maxRange * 2.0f;

    }
    else if (transform[3][1] > cameraPosition.y + maxRange)
    {
        particleData[updateIndex].startPosition.y = transform[3][1] - maxRange * 2.0f;
    }

    if (transform[3][2] < cameraPosition.z - maxRange)
    {
        particleData[updateIndex].startPosition.z = transform[3][2] + maxRange * 2.0f;
    }
    else if (transform[3][2] > cameraPosition.z + maxRange)
    {
        particleData[updateIndex].startPosition.z = transform[3][2] - maxRange * 2.0f;
    }

    instanceData[updateIndex].colour.w = clamp(cos(elapsedTime + particleData[updateIndex].fadeSpeed) * 0.5 + 0.5, 0.0, particleData[updateIndex].colour.w);
}