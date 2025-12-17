import Core;
import Material;
import Utils;
import Light;

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData2> _globalData;

[[vk::binding(1, 0)]]
Sampler2D _inputeTextureFog;

struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
    float2 screenSize : SCREEN_SIZE;
};

[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    // draw indexed with 3 vertices, requires no VAO binding
    var parsed = _globalData.Parse();
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    output.screenSize = parsed.screenSize; 
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) : SV_Target {
    return float4( _inputeTextureFog.Sample(fsIn.uv));
}