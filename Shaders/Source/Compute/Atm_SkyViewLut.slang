// based on https://github.com/MatejSakmary/atmosphere-bac/blob/rewrite/shaders/multiscatteringLUT.glsl
import Atmosphere;
import AtmosphereParameters;
import Light;
import Intersections;
import Core;
import Utils;

[[vk::binding(0,0)]]
Sampler2D transmitanceLut;

[[vk::binding(1,0)]]
Sampler2D multiscatteringLut;

[[vk::binding(2,0)]]
RWTexture2D skyViewLut;

[[vk::binding(3,0)]]
ConstantBuffer<LightInfo> light;

[[vk::binding(4,0)]]
ConstantBuffer<GlobalData> globalData;

[vk::push_constant]
AtmosphereParamsCPU atmosphereParamsCPU;

const static float cameraScale = 0.1;

struct ScatteringSample{
    float3 Mie;
    float3 Ray;
    // ozone does not scatter for now 
}

float cornetteShanksMiePhaseFunction(float g, float cosTheta) {
  float k = 3.0 / (8.0 * PI) * (1.0 - g * g) / (2.0 + g * g);
  return k * (1.0 + cosTheta * cosTheta) /
         pow(1.0 + g * g - 2.0 * g * -cosTheta, 1.5);
}

float rayleighPhase(float cosTheta) {
  float factor = 3.0 / (16.0 * PI);
  return factor * (1.0 + cosTheta * cosTheta);
}

float3 getMultipleScattering(AtmosphereParamsGPU atmosphere, float3 worldPosition, float zenithCosAngle){
    float width,height;
    if(atmosphere.UseMultiscattering){

        multiscatteringLut.GetDimensions(width, height);
        
        float2 uv = saturate(float2(zenithCosAngle*0.5f + 0.5f, (length(worldPosition) - atmosphere.radiusBottom) / (atmosphere.radiusTop - atmosphere.radiusBottom)));
        uv = float2(fromUnitToSubUvs(uv.x, width), fromUnitToSubUvs(uv.y, height));
        
        float3 multiScatteredLuminance = multiscatteringLut.Sample(uv).rgb;
        return multiScatteredLuminance;
    }
    return {0.0, 0.0, 0.0};

}

float3 integrateScatteredLuminance(AtmosphereParamsGPU atmosphere, float3 worldPosition ,float3 worldDirection, float3 sunDirection, int sampleCount){
    // how far the ray will travel
    float integrationlength = ComputeIntegrationLength(atmosphere, worldPosition, worldDirection);

    if(integrationlength <= 0) {return {1.0, 0.0, 0.0};}

    float cosTheta = dot(sunDirection, worldDirection);
    float miePhaseValue = cornetteShanksMiePhaseFunction(atmosphere.miePhaseG, -cosTheta);
    float rayLeightPhaseValue = rayleighPhase(cosTheta);

    float3 accumTrans = float3(1.0);
    float3 accumLight = float3(0.0);

    for(int i = 0; i< sampleCount; i++){
        float step_0 = float(i) / sampleCount;
        float step_1 = float(i + 1) / sampleCount;

        step_0 *= step_0;
        step_1 *= step_1;

        step_0 = step_0 * integrationlength;
        step_1 = step_1 > 1.0 ? integrationlength : step_1 * integrationlength;

        float integrationStep = step_0 + (step_1 - step_0) * 0.3;
        float dIntStep = step_1 - step_0;

        float3 newPos = worldPosition + integrationStep * worldDirection;
        
        ScatteringSample mediumScattering;
        SampleMediumScattering(atmosphere, newPos, mediumScattering.Mie, mediumScattering.Ray);
        float3 mediumExctinciton = SampleMediumExctinction(atmosphere, worldPosition );

        // the shift changes the angle to the sun and we have to recalcualte here 
        float3 upVector = normalize(newPos);
                                
        // the lut params for Transmitance texture look up,
        // they are altitude and zenith angle, zenit angle between vertical vector and sun direction
        float2 transLutParams = float2(length(newPos), dot(sunDirection, upVector));
        
        // get coordinates to sample from transmitance texture calculated earliear 
        float2 transmitanceUV = GetTransmittanceTextureUvFromRMu(atmosphere, transLutParams.x, transLutParams.y);
        
        // if we didnt have it precompute we would have to ray march towards sun as well 
        float3 transmitanceToSun = transmitanceLut.Sample(transmitanceUV).xyz;

        // get how much light is traveling in the direction we are interested in by multiplying scattered light with phase functino
        // L_in_scat = p * L_scate (in scattered light formula)
        float3 phaseTimesScattering = mediumScattering.Mie * miePhaseValue + mediumScattering.Ray * rayLeightPhaseValue;

        float earthIntersectionDistance = raySphereIntersectNearest(newPos, sunDirection, 0.0 + 0.001 * upVector, atmosphere.radiusBottom);
        float inEarthShadow = earthIntersectionDistance == -1.0 ? 1.0 : 0.0;

        // The multiscattering is an issue
        float3 multiscatteredLuminance =  getMultipleScattering(atmosphere, newPos, dot(sunDirection, upVector));


        float3 sunLight = inEarthShadow * transmitanceToSun * phaseTimesScattering + multiscatteredLuminance * (mediumScattering.Ray + mediumScattering.Mie);

        // absorption
        float3 transIncreaseOverIntegraionStep = exp(-(mediumExctinciton * dIntStep));

        float3 sunLightInteg = (sunLight - sunLight * transIncreaseOverIntegraionStep) / mediumExctinciton;

        accumLight += accumTrans * sunLightInteg;
        accumTrans *= transIncreaseOverIntegraionStep;
    }
    return accumLight;

}

[shader("compute")]
[numthreads(16, 16, 1)]
void computeMain(int3 threadIdx: SV_DispatchThreadID){
    
    float width, height;
    skyViewLut.GetDimensions(width, height);
    
    AtmosphereParamsGPU atmosParams = atmosphereParamsCPU.Parse();

    float2 atmosphereBoundaries = float2(atmosParams.radiusBottom, atmosParams.radiusTop);
    const float camearHeight = globalData.cameraPostiion.y * cameraScale;

    float3 sunDirection = light.dirLight.direction.xyz;
    // in papaer this is with .z up, however this uses .y up 
    float3 worldPosition = float3(0.0, camearHeight, 0.0 ) + float3(0.0, atmosParams.radiusBottom,0.0);

    float2 uv = float2(threadIdx.xy) / float2(width, height);
    float2 LUTParams = UvToSkyViewLUTParams(
        uv, atmosphereBoundaries, float2(width,height), length(worldPosition)
    );
    
    float sunZenithCosAngle = dot(normalize(worldPosition), sunDirection);
    float3 localSunDirection = normalize(float3(
        SafeSqrt(1.0 - sunZenithCosAngle * sunZenithCosAngle),
        sunZenithCosAngle, 0.0
        ));

    float viewZenitSinAngle = SafeSqrt(1.0 - LUTParams.x * LUTParams.x);
    
    /*
    float3 worldDirection = float3(cos(LUTParams.y) * sin(LUTParams.x),
           sin(LUTParams.y) * sin(LUTParams.x), cos(LUTParams.x));
    */

    float3 worldDirection = float3(
        sin(LUTParams.x) * cos(LUTParams.y),
        cos(LUTParams.x),
        sin(LUTParams.x) * sin(LUTParams.y)
    );
    if(!moveToTopAtmosphere(worldPosition, worldDirection, atmosphereBoundaries)){
        // we do not intersetc atmosphere, thus we store nothign
        skyViewLut[threadIdx.xy] = float4(float3(0.4), 1.0);
        return;
    }
    float3 Luminance = integrateScatteredLuminance(atmosParams, worldPosition, worldDirection, sunDirection, 40);
    
    skyViewLut[threadIdx.xy] = float4(Luminance, 1.0);
}