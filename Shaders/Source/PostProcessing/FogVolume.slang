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
ConstantBuffer<LightInfo> _light; ;


[[vk::binding(0, 1)]]
ConstantBuffer<FogVolumeParameters> _fogVolumeParametres;

struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
};

[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    // draw indexed with 3 vertices, requires no VAO binding
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);

    return output;
}

float3 ApproximateFog(RayDesc ray,float T,float shadow, float3 pixelColour){
    
    float sunAmmount = max(dot(ray.Direction, normalize(_light.dirLight.direction.xyz)), 0.0);

    float3  fogColor  = lerp( _fogVolumeParametres.fogColour.xyz, // blue
                           _light.dirLight.intensity.xyz, // yellow
                           pow(sunAmmount,8) );

    
    
    float3 finalColour;
    if(shadow > 0.1){
        finalColour = pixelColour;
    }else{

        finalColour =  T * pixelColour + (1 - T) * fogColor;
    }
    

    return finalColour;
}


[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) : SV_Target {

    float3 worldPos = _inputTexture_WorldPosition.Sample(fsIn.uv).xyz;

    RayDesc ray;
    ray.Direction = normalize(worldPos - _globalData.cameraPostiion.xyz);
    ray.Origin = _globalData.cameraPostiion.xyz;
    ray.TMin = 0.001;
    ray.TMax = distance(worldPos ,_globalData.cameraPostiion.xyz);

    float3 pixelColour = _inputTexture_SceneColour.Sample(fsIn.uv).xyz;

    // calculate transmission using beers law
    float T = exp(-ray.TMax * _fogVolumeParametres.absorption);
    
    float shadow = _inputTexture_ShadowMap.Sample(fsIn.uv).x;

    float3 finalColour;
    if(_fogVolumeParametres.rayMarched){ // ray march fog 
        finalColour = float3(0.5, 0.0, 0.5);
    }else{
        finalColour = ApproximateFog(ray, T, shadow, pixelColour);
    }

    return float4( finalColour, 1.0);

}
