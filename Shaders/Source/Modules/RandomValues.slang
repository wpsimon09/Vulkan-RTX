module RandomValues;

#define PI 3.14159265359

public float RandomValue(inout uint state) {
    state = state * 747796405 + 2891336453;
    uint result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    result = (result >> 22) ^ result;
    return float(result) / 4294967295.0;
}

public float RandomValueNormalDistribution(inout uint state) {
    float theta = 2 * 3.1415926 * RandomValue(state);
    float rho = sqrt(-2 * log(RandomValue(state)));
    return rho * cos(theta);
}

public float3 RandomDirection(inout uint state) {

    float x = RandomValueNormalDistribution(state);
    float y = RandomValueNormalDistribution(state);
    float z = RandomValueNormalDistribution(state);
    return normalize(float3(x, y, z));
}

public float3 RandomHemisphereDirection(float3 normal, inout uint rngState)
{
    float3 dir = RandomDirection(rngState);
    return dir * sign(dot(normal, dir));
}

public float2 RandomPointInCircle(inout uint rngState) {
    float angle = RandomValue(rngState) + 2 * PI;
    float2 pointOnCircle = float2(cos(angle), sin(angle));

    return pointOnCircle * sqrt(RandomValue(rngState));
}

public uint tea(in uint val0, in uint val1)
{
    uint v0 = val0;
    uint v1 = val1;
    uint s0 = 0;

    for (uint n = 0; n < 16; n++)
    {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }

    return v0;
}