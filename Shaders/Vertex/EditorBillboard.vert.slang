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
     float4x4 projection;
     float4x4 view;
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

[[vk::binding(1,0)]]
ConstantBuffer<PermodelData> perModelData ;

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    const float BillboardScale = 20.f;

    // sotre for later
    float3 vertexPos = VAO.inPosition;
    float3 normal = VAO.inNormal;
    float2 uv = VAO.inUV;

    float3 FragmentWorldPos = mul(perModelData.modelMatrix, float4(vertexPos, 1.0)).xyz;

    float3 NormalWorldPos = mul(perModelData.normalMatrix, float4(normal, 1.0)).xyz;
    
    float3 CameraForwardWorld = normalize(globalData.cameraPostiion.xyz - perModelData.positionWorldSpace);
    float3 CameraUpWorld = { 0, 1, 0};
    float3 CameraRightWorld = normalize(cross(CameraUpWorld, CameraForwardWorld));
    float3 Up = cross(CameraForwardWorld, CameraRightWorld);

    float3 vertexPosWorldSpace = perModelData.positionWorldSpace 
    + (CameraRightWorld * vertexPos.x * BillboardScale) 
    + (CameraUpWorld * vertexPos.y * BillboardScale);

    VertexOutput out;
    out.vertexData.cameraPos = globalData.cameraPostiion.xyz;
    out.vertexData.fragPos = vertexPosWorldSpace;
    out.vertexData.lightPos = globalData.lightPosition.xyz;
    out.vertexData.normal = NormalWorldPos;
    out.vertexData.uv = uv;

    float4x4 viewProj = mul(globalData.view, globalData.projection);
    out.sv_position = mul(viewProj, float4(vertexPosWorldSpace, 1.0));

    return out;
}
