import Core;
import Material;
import Utils;

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



[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) : SV_Target {

    float fogMin = 0.1;
    float fogMax = 1.0;

    float3 worldPos = _inputTexture_WorldPosition.Sample(fsIn.uv).xyz;
    float3 fogColour = float3(0.2, 0.2, 0.2);

    RayDesc ray;
    ray.Direction = normalize(worldPos - _globalData.cameraPostiion.xyz);
    ray.Origin = _globalData.cameraPostiion.xyz;
    ray.TMin = 0.001;
    ray.TMax = distance(worldPos ,_globalData.cameraPostiion.xyz);

    // calculate transmission using beers law
    float T = exp(-ray.TMax * _fogVolumeParametres.absorption);


    float3 pixelColour = _inputTexture_SceneColour.Sample(fsIn.uv).xyz;
    
    return float4( (float3(T)  ), 1.0);
}
