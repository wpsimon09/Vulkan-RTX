// based on https://github.com/MatejSakmary/atmosphere-bac/blob/rewrite/shaders/multiscatteringLUT.glsl
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

/*
Samples the medium extinction (i.e., absorption) for each component of the atmosphere.
Calculates how much light is absorbed by Mie particles, Rayleigh particles, and ozone at a given world position.
Returns the total extinction as the sum of all components.
*/
float3 SampleMediumExctinction(AtmosphereParamsGPU atmosphere,float3 worldPosition){
    const float viewHeight = length(worldPosition) - atmosphere.radiusBottom;

    const float densityMie = exp(atmosphere.mieDensity.layers[1].expScale * viewHeight);
    const float densityRei = exp(atmosphere.rayleightDensity.layers[1].expScale * viewHeight);
    const float densityOzo = atmosphere.getOzoneDensity(viewHeight); 

    float3 mieExtinction = atmosphere.mieExtinction * densityMie;
    float3 rayExtinction = atmosphere.rayLeightScattering * densityRei;
    float3 ozoneExctinction = atmosphere.absorptionExctinction * densityOzo;

    return mieExtinction + rayExtinction + ozoneExctinction;
}

/*
Samples the total scattering of light in the atmosphere, independent of direction.
Calculates the contribution from Mie and Rayleigh scattering at a given world position.
Ozone only absorbs light and does not contribute to scattering.
Returns the sum of all scattering components.
*/
float3 SampleMediumScattering(AtmosphereParamsGPU atmosphere, float3 worldPostion){
    const float viewHeight = length(worldPostion) - atmosphere.radiusBottom;

    const float densityMie = exp(atmosphere.mieDensity.layers[1].expScale * viewHeight);
    const float densityRei = exp(atmosphere.rayleightDensity.layers[1].expScale * viewHeight);
    const float densityOzo = atmosphere.getOzoneDensity(viewHeight); 

    float3 mieScattering = atmosphere.mieScattering * densityMie;
    float3 rayleighScattering = atmosphere.rayLeightScattering * densityRei;
    float3 ozoneScattering = float3(0.0f); // does not scatter light only absorbs

    return mieScattering + rayleighScattering + ozoneScattering;
}

RayMarchResult IntegrateScatteredLuminance(float3 worldPosition, float3 worldDirection, float32_t1x2)


