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

    
    column_major float4x4 rotView;
    rotView[0] = float4(globalData.view[0].xyz, 0.0);
    rotView[1] = float4(globalData.view[1].xyz, 0.0);
    rotView[2] = float4(globalData.view[2].xyz, 0.0);
    rotView[3] = float4(0.0, 0.0, 0.0, 1.0);
    
    
    //float4x4 proj = globalData.projection;

    column_major float4x4 viewProj = mul(globalData.projection, rotView);
    float4 pos = mul(viewProj, float4(VAO.inPosition, 1.0));
    out.sv_position = pos.xyww;

    return out;
}   
