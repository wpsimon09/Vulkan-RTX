// based on https://github.com/MatejSakmary/atmosphere-bac/blob/rewrite/shaders/multiscatteringLUT.glsl
import AtmosphereParameters;
import Utils;
import Intersections;
import Atmosphere;


Sampler2D TransmitanceLUT;
RWTexture2D MultiScatOutput;

[vk::push_constant]
AtmosphereParamsCPU atmosParams;

static const float PI = 3.1415926535897932384626433832795;
static const int SampleCount = 20;
static const int SphereSamples = 64;
static const float uniformPhase = 1.0 / (4.0 * PI);
static const float golden_ration = 1.6180339;
static const float planetRadiusOffset = 0.01;

groupshared float3 MultiscattSharedMem[64];
groupshared float3 LShaderMem[64];
struct RayMarchResult{
    float3 luminance;
    float3 Multiscattering;
}

float fromSubUvsToUnit(float u, float resolution) {
	return (u - 0.5 / resolution) * (resolution / (resolution - 1.0)); 
}

float fromUnitToSubUvs(float u, float resolution) {
	 return (u + 0.5f / resolution) * (resolution / (resolution + 1.0));
}


/*
Samples the medium extinction (i.e., absorption) for each component of the atmosphere.
Calculates how much light is absorbed by Mie particles, Rayleigh particles, and ozone at a given world position.
Returns the total extinction as the sum of all components.
*/
float3 SampleMediumExctinction(AtmosphereParamsGPU atmosphere,float3 worldPosition){
    const float viewHeight = length(worldPosition) - atmosphere.radiusBottom;

    // get the denity of the medium in the given height 
    const float densityMie = exp(atmosphere.mieDensity.layers[1].expScale * viewHeight);
    const float densityRei = exp(atmosphere.rayleightDensity.layers[1].expScale * viewHeight);
    const float densityOzo = atmosphere.getOzoneDensity(viewHeight); 

    // get exctincion coeficient of differnet particales in the atmosphere and scaled it based on the height 
    float3 mieExtinction = atmosphere.mieExtinction * densityMie;
    float3 rayExtinction = atmosphere.rayLeightScattering * densityRei;
    float3 ozoneExctinction = atmosphere.absorptionExctinction * densityOzo;

    return mieExtinction + rayExtinction + ozoneExctinction; // total atentuation of the energy 
}

/*
Samples the total scattering of light in the atmosphere, independent of direction.
Calculates the contribution from Mie and Rayleigh scattering at a given world position.
Ozone only absorbs light and does not contribute to scattering.
Returns the sum of all scattering components.
*/
float3 SampleMediumScattering(AtmosphereParamsGPU atmosphere, float3 worldPostion){
    const float viewHeight = length(worldPostion) - atmosphere.radiusBottom;

    // get the denity of the medium in the given height 
    const float densityMie = exp(atmosphere.mieDensity.layers[1].expScale * viewHeight);
    const float densityRei = exp(atmosphere.rayleightDensity.layers[1].expScale * viewHeight);
    const float densityOzo = atmosphere.getOzoneDensity(viewHeight); 

    // get exctincion coeficient of differnet particales in the atmosphere and scaled it based on the height 
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
        /* Ray missies both atmosphere and ground so we are not gonna ray march*/
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
    // to calculate the multiple scattering 
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
        
        // if we didnt have it precompute we would have to ray march towards sun as well 
        float3 transmitanceToSun = TransmitanceLUT.Sample(transmitanceUV).xyz;

        // how much light is scattered (regardless of dimension) (betaS or sigma S )
        float3 mediumScattering = SampleMediumScattering(atmosphere,newPos);

        // how much light is scattered and abbsorbed (beta T or Sigma T )
        float3 mediumExctinction = SampleMediumExctinction(atmosphere, newPos);

        float3 transIncreaseOverIntegraionStep = exp(-(mediumExctinction * integrationStep));

        //TODO: this can be made just bool function
        float earthIntersectionDistance = raySphereIntersectNearest(newPos, sunDireciton, planet0 + planetRadiusOffset * upVector, atmosphere.radiusBottom);
        float inEarthShadow = earthIntersectionDistance == -1.0 ? 1.0 : 0.0;

        // ammount of radiation comming from the light 
        // I = Vis * T(x, sun) * Beta_s(at height) * p
        float3 sunLight = inEarthShadow * transmitanceToSun * mediumScattering * uniformPhase;

        float3 multiscatteriedContInt = (mediumScattering - mediumScattering * transIncreaseOverIntegraionStep) / mediumExctinction;
        float3 inscatteerdContInt = (sunLight - sunLight * transIncreaseOverIntegraionStep) / mediumScattering;

        result.Multiscattering += accumTrans * multiscatteriedContInt;
        accumLight += accumTrans * inscatteerdContInt;

        accumTrans *= transIncreaseOverIntegraionStep;
    }
    result.luminance = accumLight;

    return result;
    // TODO: account for the light bouncing from the earht and add it to the result 
}


[shader("compute")]
[numthreads(1, 1, 64)]
void computeMain(int3 threadIdx: SV_DispatchThreadID){
    
    int width, height;
    MultiScatOutput.GetDimensions(width, height);
    
    AtmosphereParamsGPU atmos = atmosParams.Parse();
    atmos.transmitanceTextureWidth = width;
    atmos.transmitanceTextureHeight = height;

    float2 uv = (float2(threadIdx.xy) + float2(0.5 , 0.5)) / float2(width, height);
    uv = float2(fromSubUvsToUnit(uv.x, width), fromSubUvsToUnit(uv.y, height));

    // map the UVs to the dimension of hte look up table 
    // x - sun zenith
    // y - height at the atmosphere 
    float sunCosZenithAngle = uv.x * 2.0 - 1.0;
    
    float3 sunDirection = float3(
      0.0, sqrt(clamp(1.0 - sunCosZenithAngle * sunCosZenithAngle, 0.0, 1.0)),
      sunCosZenithAngle);

    float viewHeight = atmos.radiusBottom +  clamp(uv.y + planetRadiusOffset, 0.0, 1.0) *
          (atmos.radiusTop -
           atmos.radiusBottom - planetRadiusOffset);

    float3 worldPosition = float3(0.0, 0.0, viewHeight);

    float sampleIdx = threadIdx.z;

    
    // generates sphere samples
    float theta = acos(1.0 - 2.0 * (sampleIdx + 0.5) / SphereSamples);
    float phi = (2 * PI * sampleIdx) / golden_ration;

    // to cartesian
    float3 worldDir = float3(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
    
    RayMarchResult result = IntegrateScatteredLuminance(atmos, worldPosition, worldDir, sunDirection, SampleCount);
    
    MultiscattSharedMem[threadIdx.z] = result.Multiscattering / SphereSamples;
    LShaderMem[threadIdx.z] = result.luminance / SphereSamples;

    GroupMemoryBarrierWithGroupSync();
    
    if(threadIdx.z < 32) {
        MultiscattSharedMem[threadIdx.z] += MultiscattSharedMem[threadIdx.z + 32];
        LShaderMem[threadIdx.z] += LShaderMem[threadIdx.z + 32];
    }
    
    GroupMemoryBarrierWithGroupSync();

    if(threadIdx.z < 16) {
        MultiscattSharedMem[threadIdx.z] += MultiscattSharedMem[threadIdx.z + 16];
        LShaderMem[threadIdx.z] += LShaderMem[threadIdx.z + 16];
    }
    
    GroupMemoryBarrierWithGroupSync();

    if(threadIdx.z < 8) {
        MultiscattSharedMem[threadIdx.z] += MultiscattSharedMem[threadIdx.z + 8];
        LShaderMem[threadIdx.z] += LShaderMem[threadIdx.z + 8];
    }
    
    GroupMemoryBarrierWithGroupSync();

    if(threadIdx.z < 4) {
        MultiscattSharedMem[threadIdx.z] += MultiscattSharedMem[threadIdx.z + 8];
        LShaderMem[threadIdx.z] += LShaderMem[threadIdx.z + 8];
    }
    
    GroupMemoryBarrierWithGroupSync();

    if(threadIdx.z < 2) {
        MultiscattSharedMem[threadIdx.z] += MultiscattSharedMem[threadIdx.z + 2];
        LShaderMem[threadIdx.z] += LShaderMem[threadIdx.z + 2];
    }
    
    GroupMemoryBarrierWithGroupSync();

    if(threadIdx.z < 1) {
        MultiscattSharedMem[threadIdx.z] += MultiscattSharedMem[threadIdx.z + 1];
        LShaderMem[threadIdx.z] += LShaderMem[threadIdx.z + 1];
    }
    
    GroupMemoryBarrierWithGroupSync();

    if(threadIdx.x == 0 ) return;

    float3 multiscattSum = MultiscattSharedMem[0];
    float3 InScattLumSum = LShaderMem[0];

    float3 r = multiscattSum;
    float3 sumOfAllMultiscatteringEventsContribution = float3(1.0 / (1.0 - r.x), 1.0 / (1.0 - r.y), 1.0 / (1.0 - r.z));

    float3 lum = InScattLumSum * sumOfAllMultiscatteringEventsContribution;

    MultiScatOutput[threadIdx.xy] = float4(lum, 1.0);

}


