// based on https://github.com/MatejSakmary/atmosphere-bac/blob/rewrite/shaders/multiscatteringLUT.glsl
import AtmosphereParameters;
import Utils;
import Intersections;
import Atmosphere;


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

RayMarchResult IntegrateScatteredLuminance(AtmosphereParamsGPU atmosphere, float3 worldPosition, float3 worldDirection, float3 sunDireciton, float sampleCount){
    RayMarchResult result = RayMarchResult();
    float3 planet0 = float3(0.0);
    float planetIntersectionDistance = raySphereIntersectNearest(
        worldPosition, worldDirection, planet0, atmosphere.radiusBottom);
    float atmosphereIntersectionDistance = raySphereIntersectNearest(
        worldPosition, worldDirection, planet0, atmosphere.radiusTop);

    float integrationLength;
    if((planetIntersectionDistance == -1.0 && atmosphereIntersectionDistance > 0.0)){
        /* Ray intersects only atmospher*/
        return result;
    }    
     else if((planetIntersectionDistance == -1.0) && (atmosphereIntersectionDistance > 0.0)){
        /* ray intersects only atmosphere */
        integrationLength = atmosphereIntersectionDistance;
    }
    else if((planetIntersectionDistance > 0.0) && (atmosphereIntersectionDistance == -1.0)){
        /* ray intersects only planet */
        integrationLength = planetIntersectionDistance;
    } else {
        /* ray intersects both planet and atmosphere -> return the first intersection */
        integrationLength = min(planetIntersectionDistance, atmosphereIntersectionDistance);
    }
    float integrationStep = integrationLength / float(sampleCount);

    float2 atmosphereBoundaries = float2(atmosphere.radiusBottom, atmosphere.radiusTop);

    // accumulated transmittance uring the ray march process
    float3 accumTrans = float3(1.0);

    // stores accumlated light contribution during ray marching
    float3 accumLight = float3(1.0);

    float oldRayShift = 0;

    //==========================
    // Ray marching

    for(int i = 0; i < sampleCount; i++){
        float newRayShift = integrationLength * (float(i) + 0.3) / float(sampleCount);
        integrationStep = newRayShift - oldRayShift;
        float3 newPos = worldPosition + newRayShift * worldDirection;
        oldRayShift = newRayShift;

        // the shift changes the angle to the sun and we have to recalcualte here 
        float3 upVector = normalize(newPos);
                                
        // the lut params for Transmitance texture look up,
        // they are altitude and zenith angle, zenit angle between vertical vector and sun direction
        float2 transLutParams = float2(length(newPos), dot(sunDireciton, upVector));
        
        // get coordinates to sample from transmitance texture calculated earliear 
        float2 transmitanceUV = GetTransmittanceTextureUvFromRMu(atmosphere, transLutParams.x, transLutParams.y);
        
        float3 transmitanceToSun = TransmitanceLUT.Sample(transmitanceUV).xyz;
        float3 mediumScattering = SampleMediumScattering(atmosphere,newPos);
        float3 mediumExctinction = SampleMediumExctinction(atmosphere, newPos);



    }
}




