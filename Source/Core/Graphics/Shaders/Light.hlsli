struct PointlightData
{
    float3 position;
    float padding1;
    float3 colour;
    float padding2;
    float intensity;
    float radius;
    float falloff;
    float padding3;
    bool active;
    float3 padding4;
};

struct SpotlightData
{
    float3 position;
    float padding1;
    float3 direction;
    float padding2;
    float3 colour;
    float intensity;
    float range;
    float innerAngle;
    float outerAngle;
    bool active;
};