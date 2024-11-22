struct VertexInput {
    float3 inPosition :POSITION ;
    float3 inNormal : NORMAL;
    float2 inUV : UV;
};

struct CoarseVertex {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
    float4 outPos : SV_Position;
};

struct VertexOutput {
    CoarseVertex vertexData : DATA;
    float4 position : SV_Position;
}

struct PermodelData {
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[vk::binding(0,0)]]
ParameterBlock<PermodelData> perModelData : register(b0); 


struct GlobalData {
    float4x4 projection;
    float4x4 view;
    float3 lightPosition;
    float3 cameraPostiion;
};

[[vk::binding(0,1)]]
ParameterBlock<GlobalData> globalData;

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // input uniform{
{
    float3 vertexPos = VAO.inPosition;
    float3 normal = VAO.inNormal;
    float2 uv = VAO.inUV;

    float3 FragmentWorldPos = mul(perModelData.modelMatrix, float4(vertexPos, 1.0)).xyz;
    float3 NormalWorldPos = mul(perModelData.normalMatrix, float4(normal, 1.0)).xyz;

    

    VertexOutput out;
    out.vertexData.cameraPos = globalData.cameraPostiion;
    out.vertexData.fragPos = FragmentWorldPos;
    out.vertexData.lightPos = globalData.lightPosition;
    out.vertexData.normal = NormalWorldPos;
    out.vertexData.uv = uv;
    float4x4 viewProj = mul(globalData.projection, globalData.view);
    out.vertexData.outPos = mul(viewProj, float4(FragmentWorldPos, 1.0));

    return out;
}
