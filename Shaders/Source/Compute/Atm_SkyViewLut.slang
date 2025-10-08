import Atmosphere;
import AtmosphereParameters;
import Light;

Sampler2D transmitanceLut;
readonly Texture2D multiscatteringLut;
RWTexture2D skyViewLut;
ConstantBuffer<LightInfo> light;

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
    multiscatteringLut.GetDimensions(width, height);

    float2 uv = clamp(float2(zenithCosAngle * 0.5 + 0.5,
                 (length(worldPosition) - atmosphere.radiusBottom) /
                     (atmosphere.radiusTop -
                      atmosphere.radiusBottom)),
            0.0, 1.0);

    uv = float2(fromUnitToSubUvs(uv.x, width), fromUnitToSubUvs(uv.y, height));
    int2 coords = int2(uv * int2(width, height));

    return multiscatteringLut[coords].rgb;
}

float3 integrateScatteredLuminance(AtmosphereParamsGPU atmosphere, float3 worldPosition ,float3 worldDirection, float3 sunDirection, int sampleCount){
    // how far the ray will travel
    float integrationlength = ComputeIntegrationLength(atmosphere, worldPosition, worldDirection);

    if(integrationlength <= 0) {return {};}

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
        SampleMediumScattering(atmosphere, worldPosition, mediumScattering.Mie, mediumScattering.Ray);
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
        float3 phaseTimesScattering = mediumScattering.Mie * miePhaseValue + mediumScattering.Ray * rayleighPhase;

        


    }

}