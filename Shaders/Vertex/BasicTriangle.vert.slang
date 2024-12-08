struct VertexInput {
    float3 inPosition :POSITION ;
    float3 inNormal : NORMAL;
    float2 inUV : UV;
};

struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;
    float4 sv_position : SV_Position;
}

struct GlobalData {
    column_major float4x4 projection;
    column_major float4x4 view;
    column_major float4x4 inverseView;

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
};

[[vk::binding(1,0)]]
ConstantBuffer<PermodelData> perModelData ; 

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    // sotre for later
    float3 vertexPos = VAO.inPosition;
    float3 normal = VAO.inNormal;
    float2 uv = VAO.inUV;

    float3 FragmentWorldPos = mul(perModelData.modelMatrix, float4(vertexPos, 1.0)).xyz;
    float3 NormalWorldPos = mul(perModelData.normalMatrix, float4(normal, 1.0)).xyz;

    VertexOutput out;
    out.vertexData.cameraPos = globalData.cameraPostiion.xyz;
    out.vertexData.fragPos = FragmentWorldPos;
    out.vertexData.lightPos = globalData.lightPosition.xyz;
    out.vertexData.normal = NormalWorldPos;
    out.vertexData.uv = uv;

    float4x4 viewProj = mul(globalData.view, globalData.projection);
    out.sv_position = mul(viewProj, float4(FragmentWorldPos, 1.0));

    return out;
}
