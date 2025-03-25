struct VertexInput {
    float3 inPosition :POSITION ;
    float3 inNormal : NORMAL;
    float2 inUV : UV;
};

struct FragmentShaderIn {
    float3 localPos : OUT_LOCAL_POS;
    float2 uv : OUT_UV;
};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;
    float4 sv_position : SV_Position;
}

struct GlobalData {
    column_major float4x4 projection;
    column_major float4x4 view;
    float4x4 inverseView;

    float4 lightPosition;
    float4 cameraPostiion;
    float4 viewParams;

    float padding;
    float padding2;
    float2 screenSize;
};

[[vk::binding(0,0)]]
ConstantBuffer<GlobalData> globalData;

struct PermodelData {
    column_major float4x4 modelMatrix;
    column_major float4x4 normalMatrix;
    float3 positionWorldSpace;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    // sotre for later
    float3 vertexPos = VAO.inPosition;
    float3 normal = VAO.inNormal;
    float2 uv = VAO.inUV;

    VertexOutput out;
    out.vertexData.localPos = vertexPos;
    out.vertexData.uv = uv;

    float4x4 rotView = float4x4(
        float4(globalData.view[0].xyz, 0.0),
        float4(globalData.view[1].xyz, 0.0),
        float4(globalData.view[2].xyz, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );

    float4x4 projView = mul(rotView, globalData.projection);
    float4 pos = mul(float4(vertexPos, 1.0), projView);
        
    out.sv_position = pos.xyww;

    return out;
}
