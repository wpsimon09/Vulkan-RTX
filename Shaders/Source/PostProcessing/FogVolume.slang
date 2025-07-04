import Core;
import Material;
import Utils;
import Light;

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> _globalData;

[[vk::binding(1, 0)]]
Sampler2D _inputTexture_ShadowMap;

[[vk::binding(2, 0)]]
Sampler2D _inputTexture_WorldPosition;

[[vk::binding(3, 0)]]
Sampler2DArray _inputTexture_BlueNoise;

[[vk::binding(4, 0)]]
Sampler2D _inputTexture_SceneColour;

[[vk::binding(5, 0)]]
ConstantBuffer<LightInfo> _light;

[[vk::binding(6, 0)]]
RaytracingAccelerationStructure _tlas;

[[vk::binding(0, 1)]]
ConstantBuffer<FogVolumeParameters> _fogVolumeParametres;

struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
    float2 screenSize : SCREEN_SIZE;
};

[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    // draw indexed with 3 vertices, requires no VAO binding
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    output.screenSize = _globalData.screenSize; 
    return output;
}

float3 ApproximateFog(RayDesc ray,float T, float T_light , float3 pixelColour, float shadow){
    
    float sunAmmount = max(dot(ray.Direction, normalize(_light.dirLight.direction.xyz)), 0.0);

    float3  fogColor  = _fogVolumeParametres.fogColour.xyz;

    // phase function
    float cos_theta = dot(normalize(ray.Direction) ,normalize(_light.dirLight.direction.xyz));
    float p = Henyey_GreensteinPhaseFunction(_fogVolumeParametres.asymetryFactor, cos_theta);

    float3 finalColour = pixelColour * T + (1-T) * fogColor + p ;

    //finalColour =  T_light * pixelColour + volumetricLight + (1 - T) * fogColor;
 
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

    float betaS = _fogVolumeParametres.sigma_s; // scatering coeficient 
    float T = exp(-t);

    return T * betaS * v * I * stepSize;
}


float3 RayMarchFog(RayDesc ray, float blueNoise, float3 pixelColour)
{
    float stepSize = ray.TMax / _fogVolumeParametres.raySteps;
    float numSteps = _fogVolumeParametres.raySteps;
    
    float sigma_t = _fogVolumeParametres.sigma_a + _fogVolumeParametres.sigma_s;
    float perStepAttentution = exp(-stepSize * sigma_t);
    
    float cos_theta = dot(normalize(ray.Direction), normalize(_light.dirLight.direction.xyz));
    float3 Result = 0.0;
    float T = 1.0;
    float p = Henyey_GreensteinPhaseFunction(_fogVolumeParametres.asymetryFactor, cos_theta);
    float3 I = _light.dirLight.intensity.xyz * _light.dirLight.intensity.w;
    
    // Integrate the in-scattering
    for (int i = 0; i < numSteps; i++)
    {
        float rayT = ray.TMin + stepSize * (float(i) + blueNoise);
        float3 samplePos = ray.Origin + ray.Direction * rayT;
        float lightTravelDistance = 4000;
        
        T *= perStepAttentution;
        bool hit = IntersectsP(InitRay(samplePos , normalize(_light.dirLight.direction.xyz), 4000), _tlas, lightTravelDistance);
        float V = hit ? 0.0 : 1.0;

        Result += Li_x(lightTravelDistance, V, stepSize, I);
    }
    
    return (pixelColour * T) + (p * Result) + ((1 - T) * _fogVolumeParametres.fogColour.xyz);
}




[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) : SV_Target {

    
    float3 worldPos = _inputTexture_WorldPosition.Sample(fsIn.uv).xyz;

    RayDesc ray;
    ray.Direction = normalize(worldPos - _globalData.cameraPostiion.xyz);
    ray.Origin = _globalData.cameraPostiion.xyz;
    ray.TMin = 0;
    // get the disnatnce that the ray will travel
    ray.TMax = min(distance(worldPos ,_globalData.cameraPostiion.xyz), _fogVolumeParametres.rayDistance);
    
    float2 shadow_t_light = _inputTexture_ShadowMap.Sample(fsIn.uv).xy;
    
    // calculate transmission using beers law
    float T = exp(-ray.TMax * (_fogVolumeParametres.sigma_s + _fogVolumeParametres.sigma_a));
    float T_light = exp(-shadow_t_light.y * (_fogVolumeParametres.sigma_s + _fogVolumeParametres.sigma_a));
    float3 pixelColour = _inputTexture_SceneColour.Sample(fsIn.uv).xyz;
    
    // get the blue noise
    float3 uvi = float3(fsIn.uv, _globalData.currentFrame % 64.0); // uv corrdintates + i - index to the texture array based on the frame 
    float blueNoise = _inputTexture_BlueNoise.Sample(uvi).x;

    float3 finalColour;
    if(_fogVolumeParametres.rayMarched){ 
        //=====================
        // ray marched fog 
        //finalColour = RayMarchFog(ray, fsIn.screenSize, pixelColour);
        finalColour = RayMarchFog(ray, blueNoise, pixelColour);

    }else{
        //====================
        // approximate fog 
        finalColour = ApproximateFog(ray, T, T_light, pixelColour, shadow_t_light.x);
    }

    return float4( finalColour, 1.0);

}



/*
* Other ray marching from the shader toy 
float3 RayMarchFog2(RayDesc ray, float blueNoise,float2 screenSize,  float3 pixelColour){
    float3 fogColorLit = _fogVolumeParametres.fogColour.xyz;
    float3 fogColorUnlit = float3(0.0f);
    float3 fogLitPercent = 0.0f;

    for(int i = 0; i < _fogVolumeParametres.raySteps; i++){
        float3 samplePos = ray.Origin + ray.Direction * float(i );
        float lightTravelDistance;
        bool hit = IntersectsP(InitRay(samplePos, _light.dirLight.direction.xyz, ray.TMax), _tlas, lightTravelDistance);
        fogLitPercent = lerp(fogLitPercent,float(hit), 1.0 / float(i+1));
    }

    float3 fogColor = lerp(fogColorLit, fogColorUnlit, fogLitPercent);
    float absorb = exp(- ray.TMax * _fogVolumeParametres.sigma_a);
    return lerp(fogColor, pixelColour, absorb);

}
*/
