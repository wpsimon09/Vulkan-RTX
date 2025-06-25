import Core;
import Material;
import Utils;

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> _globalData;

[[vk::binding(1, 0)]]
Sampler2D _inputTexture_ShadowMap;

[[vk::binding(2, 0)]]
Sampler2D _inputTexture_Depth;

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

    float depth = _inputTexture_Depth.Sample(fsIn.uv).x;
    float linearDepht = LinearizeDepth(depth, _globalData.viewParams.z, _globalData.viewParams.w);

    float shadowFactor = _inputTexture_ShadowMap.Sample(fsIn.uv).x;
    if (shadowFactor < 1.0) {
        return float4(1.0, 0.0, 0.0, _fogVolumeParametres.density);
    } else {
        return float4(0.0, 1.0, 0.0, 1.0);
    }
}
