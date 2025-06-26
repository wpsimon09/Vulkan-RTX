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

[[vk::binding(0, 1)]]
ConstantBuffer<FogVolumeParameters> _fogVolumeParametres;

[[vk::binding(1, 1)]]
ConstantBuffer<LightInfo> _light;

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

float3 fogDensity(float3 bg, float t, float3 v, float3 sunDir) {
    float sigma_t = _fogVolumeParametres.sigma_t(); // get the scattering coeficient 
    float fogAmmount  = 1.0 - exp(-t * sigma_t); // beer`s law Transmitance = exp(-distance_in_volume * coeficient)

    float sunAmmount = max(dot(normalize(v), normalize(sunDir)), 0.0);
    
    float3 fogColour = lerp(float3(0.5, 0.6, 0.7), _light.dirLight.intensity.xyz, pow(sunAmmount, _light.dirLight.intensity.w));

    float3 finalColour = bg * exp(-t * sigma_t)  + fogColour* (1.0 - exp(-t * sigma_t));

    return lerp(bg, finalColour, fogAmmount);
}

[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) : SV_Target {

    float3 worldPos = _inputTexture_WorldPosition.Sample(fsIn.uv).xyz;
    float3 V = worldPos - _globalData.cameraPostiion.xyz;
    float t = distance(worldPos, _globalData.cameraPostiion.xyz);
    float3 bg = _inputTexture_SceneColour.Sample(fsIn.uv).xyz;
    


    return float4(fogDensity(bg, t, V, _light.dirLight.direction.xyz), 1.0);
}
