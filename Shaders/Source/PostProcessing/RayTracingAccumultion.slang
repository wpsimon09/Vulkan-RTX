struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
};

ConstantBuffer<Sampler2D> _previousRender;
ConstantBuffer<Sampler2D> _currentRender;


[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOut in) {
    return float4(0.0f, 0.0f, 0.0, 1.0);
}