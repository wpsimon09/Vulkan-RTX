import Core;
import Material;
import Utils;
import Light;

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> _globalData;

[[vk::binding(2, 0)]]
Sampler2D _inputTexture_WorldPosition;

[[vk::binding(3, 0)]]
Sampler2D _inputTexture_BlueNoise;

[[vk::binding(4, 0)]]
ConstantBuffer<LightInfo> _light;

[[vk::binding(5, 0)]]
RaytracingAccelerationStructure _tlas;

[[vk::binding(6, 0)]]
RWTexture2D _outputFog; 

[[vk::push_constant]]
FogVolumeParameters _fogVolumeParametres;

float4 ApproximateFog(RayDesc ray,float T ,float worldY){
    
    float sunAmmount = max(dot(ray.Direction, normalize(_light.dirLight.direction.xyz)), 0.0);

    float3  fogColor  = _fogVolumeParametres.fogColour.xyz;

    // phase function
    float cos_theta = dot(normalize(ray.Direction) ,normalize(_light.dirLight.direction.xyz));
    float p = Henyey_GreensteinPhaseFunction(_fogVolumeParametres.asymetryFactor, cos_theta);
    float3 E_sun = _light.dirLight.intensity.xyz * _light.dirLight.intensity.w;
    float beta_S = _fogVolumeParametres.sigma_s;

    float heightFactor = exp(-_fogVolumeParametres.heightFallOff * (worldY - _fogVolumeParametres.fogHeight));

    // analitical solution of the inscattering integral by Hoffman and Preetham
    float3 in_scattering =  (E_sun * p) / beta_S * (1-exp(-beta_S * ray.TMax)) * heightFactor; 

    T = exp(-beta_S * heightFactor * ray.TMax);

    float4 finalColour;
    finalColour.xyz = T +  in_scattering  + (1-T) * fogColor ;
    finalColour.w = T;

    return finalColour;
}

// Physically based rendering, 11.2
float Henyey_GreensteinPhaseFunction(float g, float cosTheta){
    float Inv4Pi  = 1.0 / (4.0 * float.getPi());

    float denom = 1.0 + g * g - 2.0 * g * cosTheta;
    return Inv4Pi * (1.0 - g * g) / (denom * sqrt(denom));
}

/**
* Calculates in scattering of the light at the sample position 
* t - how much did light travel through the volume 
* cosTheta - angle betweeen light and view ray
* v - visibility 
* stepSize - size of the step (dx) in the integra 
* I - light intensity 
**/
float3 Li_x(float t, float v, float stepSize, float3 I ) { 
    return  _fogVolumeParametres.sigma_s * v * I * stepSize;
}

float EvaluateHeightDensity(float3 world){
    float baseFog = _fogVolumeParametres.fogHeight;
    float fogFallOff = _fogVolumeParametres.heightFallOff;

    float density = exp(-max(0, world.y - baseFog) * fogFallOff); // exponentially decrease fog 
    
    return density;
}

float4 RayMarchFog(RayDesc ray,float3 worldPos, float blueNoise)
{
    float stepSize = ray.TMax / _fogVolumeParametres.raySteps;
    float numSteps = _fogVolumeParametres.raySteps;
        
    
    float3 In_Scattering = 0.0;
    float T = 1.0;
    float3 I = _light.dirLight.intensity.xyz * _light.dirLight.intensity.w;
    
    float density = (_fogVolumeParametres.sigma_a + _fogVolumeParametres.sigma_s) * _fogVolumeParametres.fogColour.w; // w - is density of volume 
    
    // Integrate the in-scattering
    for (int i = 0; i < numSteps; i++)
    {
        float rayT = ray.TMin + stepSize * (float(i) + blueNoise);
        float3 samplePos = ray.Origin + ray.Direction * rayT;
        
        float heightAttentuation = EvaluateHeightDensity(samplePos);
        
        float sigma_t = density * heightAttentuation;
        float perStepAttenuation = exp(-stepSize * sigma_t);

        if((bool)_light.dirLight.parameters.z){

            float lightTravelDistance;
            bool hit = IntersectsP(InitRay(samplePos, normalize(_light.dirLight.direction.xyz), 4000), _tlas, lightTravelDistance);
            float v = hit ? 0.0 : 1.0;
    
            float3 V = normalize(samplePos - _globalData.cameraPostiion.xyz );
            float cos_theta = dot(V, normalize(_light.dirLight.direction.xyz));
            float p = Henyey_GreensteinPhaseFunction(_fogVolumeParametres.asymetryFactor, cos_theta);
    
            In_Scattering += T * p * _fogVolumeParametres.sigma_s * v * I * stepSize * heightAttentuation * _fogVolumeParametres.fogColour.xyz ;
        }

        T *= perStepAttenuation;
        
    }

    float4 finalColour;
    finalColour.xyz = T + In_Scattering + (1-T); 
    finalColour.w = T;
    return finalColour;
}

[shader("compute")]
[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID :SV_DispatchThreadID){
    float2 dstWriteCoords = float2(threadID.xy);
    float2 dims;
    _outputFog.GetDimensions(dims.x, dims.y);

    float2 texCoords = (dstWriteCoords + 0.5) / dims;

    float3 worldPos = _inputTexture_WorldPosition.Sample(texCoords).xyz;


    RayDesc ray;
    ray.Direction = normalize(worldPos - _globalData.cameraPostiion.xyz);
    ray.Origin = _globalData.cameraPostiion.xyz;
    ray.TMin = 0.001;
    // get the disnatnce that the ray will travel
    ray.TMax = min(distance(worldPos ,_globalData.cameraPostiion.xyz), 10000);
        
    // calculate transmission using beers law
    float T = exp(-ray.TMax * (_fogVolumeParametres.sigma_s + _fogVolumeParametres.sigma_a));
    
    float blueNoise = _inputTexture_BlueNoise.Sample(texCoords).x ;

    float4 finalColour;
    if(_fogVolumeParametres.rayMarched){ 
        //=====================
        // ray marched fog 
        //finalColour = RayMarchFog(ray, fsIn.screenSize, pixelColour);
        finalColour = RayMarchFog(ray,worldPos, blueNoise);
    }else{
        //====================
        // approximate fog 
        finalColour = ApproximateFog(ray, T, worldPos.y);
    }

    _outputFog[threadID.xy] = finalColour;
}

