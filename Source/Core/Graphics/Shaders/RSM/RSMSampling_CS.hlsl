#define PI (3.141592653)
#define TWO_PI (2.0 * PI)

cbuffer HammersleyData : register (b0)
{
    uint sampleCount;
}

// Reverses the bits of the input
uint BitfieldReverse(const uint aBits)
{
    uint b = (uint(aBits) << 16u) | (uint(aBits) >> 16u);
    b = (b & 0x55555555u) << 1u | (b & 0xAAAAAAAAu) >> 1u;
    b = (b & 0x33333333u) << 2u | (b & 0xCCCCCCCCu) >> 2u;
    b = (b & 0x0F0F0F0Fu) << 4u | (b & 0xF0F0F0F0u) >> 4u;
    b = (b & 0x00FF00FFu) << 8u | (b & 0xFF00FF00u) >> 8u;
    return b;
}

float2 Hammersley(const uint aI, const uint aN)
{
    return float2(float(aI) / float(aN), float(BitfieldReverse(aI)) * 2.3283064365386963e-10);
}

RWStructuredBuffer<float2> HammersleyBuffer : register(u0);

[numthreads(600, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    if (dispatchThreadId.x < sampleCount)
    {
        // Calculate Hammersley offset for this thread
        HammersleyBuffer[dispatchThreadId.x] = Hammersley(dispatchThreadId.x, sampleCount);
    }
}