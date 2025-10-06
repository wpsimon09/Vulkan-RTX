import AtmosphereParameters;
import Utils;

Sampler2D TransmitanceLUT;
RWTexture2D MultiScatOutput;

[vk::push_constant]
AtmosphereParamsCPU atmosParams;

static const float PI = 3.14;
static const int sqrtSampleCount = 8;
static const int SphereSamples = 64;
static const float uniformPhase = 1.0 / (4.0 * PI);
static const float golden_ration = 1.6180339;

groupshared float3 MultiscattSharedMem[64];
groupshared float3 LShaderMem[64];

struct RayMarchResult{
    float3 luminance;
    float3 Multiscattering;
}

float3 SampleMediumExctinction(AtmosphereParamsGPU atmosphere,float3 worldPosition){
    const float viewHeight = length(worldPosition) - atmosphere.radiusBottom;

    const float densityMie = exp(atmosphere.mieDensity[1])
}

