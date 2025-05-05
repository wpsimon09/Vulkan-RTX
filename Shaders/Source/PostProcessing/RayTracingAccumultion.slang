struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
};

struct GlobalData {
    column_major float4x4 projection;
    column_major float4x4 view;
    float4x4 inverseView;
    float4x4 invProjection;

    float4 lightPosition;
    float4 cameraPostiion;
    float4 viewParams; // xy- camera plane width and height,z - nerPlane, w - focal length

    float raysPerPixel;
    float reccursionDepth;
    float2 screenSize;
    float numberOfFrames;
};

[[vk::binding(3, 0)]]
ConstantBuffer<Sampler2D> _previousRender;

[[vk::binding(4, 0)]]
ConstantBuffer<Sampler2D> _currentRender;


[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    // draw indexed with 3 vertices, requires no VAO binding
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOut in) {
    return float4(0.0f, 0.0f, 0.0, 1.0);
}