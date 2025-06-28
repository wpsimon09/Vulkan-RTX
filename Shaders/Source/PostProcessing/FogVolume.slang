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
Sampler2D _inputTexture_BlueNoise;

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
        
    float3 volumetricLight = T * T_light * _fogVolumeParametres.sigma_s * _light.dirLight.intensity.xyz ;
    
    float3 finalColour;
    finalColour =  T_light * pixelColour + volumetricLight + (1 - T) * fogColor;
 

    return finalColour;
}

float3 Li_x(float t, float stepSize, float3 col ) { 

    return (exp(-t * _fogVolumeParametres.sigma_s) * col) * stepSize; 
}

float3 RayMarchFog(RayDesc ray,float2 screenSize, float lightT, float3 pixelColour){

    float stepSize = ray.TMax / _fogVolumeParametres.raySteps; // dx
    float numSteps = _fogVolumeParametres.raySteps;
    float perStepAttentution = exp(-stepSize * _fogVolumeParametres.sigma_a);
    float T = 1.0; // fog is fully transisive at the begining 

    float3 Li = 0.0; // there is no light at the begining of the sample 
    // march the ray
    for(int i = 0; i < numSteps; i++){
        // calculate sample position in the world
        float rayT = ray.TMax - stepSize * (i + 0.5);
        
        float3 samplePos = ray.Origin + rayT * ray.Direction;
        
        float lightTravelDistance;

        IntersectsP(InitRay(samplePos, _light.dirLight.direction.xyz, 1000), _tlas, lightTravelDistance);

        // backward ray marching first
        T *= perStepAttentution;
        
        Li += Li_x(lightTravelDistance, stepSize, _light.dirLight.intensity.xyz);

        Li *= perStepAttentution;
    }

    return pixelColour * T + Li;
}


[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) : SV_Target {

    float3 worldPos = _inputTexture_WorldPosition.Sample(fsIn.uv).xyz;

    RayDesc ray;
    ray.Direction = normalize(worldPos - _globalData.cameraPostiion.xyz);
    ray.Origin = _globalData.cameraPostiion.xyz;
    ray.TMin = 0.001;
    // get the disnatnce that the ray will travel
    ray.TMax = min(distance(worldPos ,_globalData.cameraPostiion.xyz), _fogVolumeParametres.rayDistance);
    
    float2 shadow_t_light = _inputTexture_ShadowMap.Sample(fsIn.uv).xy;
    
    
    // calculate transmission using beers law
    float T = exp(-ray.TMax * _fogVolumeParametres.sigma_s + _fogVolumeParametres.sigma_a);
    float T_light = exp(-shadow_t_light.y * (_fogVolumeParametres.sigma_s + _fogVolumeParametres.sigma_a));
    float3 pixelColour = _inputTexture_SceneColour.Sample(fsIn.uv).xyz;
    
    
    float3 finalColour;
    if(_fogVolumeParametres.rayMarched){ // ray march fog 
        finalColour = RayMarchFog(ray, fsIn.screenSize, shadow_t_light.y, pixelColour);
    }else{
        //====================
        // approximate fog 
        finalColour = ApproximateFog(ray, T, T_light, pixelColour, shadow_t_light.x);
    }

    return float4( finalColour, 1.0);

}
