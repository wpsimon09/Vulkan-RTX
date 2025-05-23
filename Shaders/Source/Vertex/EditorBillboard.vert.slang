import Utils;

struct VertexInput {
    float3 inPosition :POSITION ;
    float2 inUV : UV;
};

struct FragmentShaderIn {
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
    float4x4 invProjection;

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

[[vk::binding(1, 0)]]
ConstantBuffer<PermodelData> perModelData ;

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    const float BillboardScale = 30.f;

    // sotre for later    
    float3 CameraForwardWorld = normalize(globalData.cameraPostiion.xyz - perModelData.positionWorldSpace);
    float3 CameraUpWorld = { 0, 1, 0};
    float3 CameraRightWorld = normalize(cross(CameraUpWorld, CameraForwardWorld));
    float3 Up = cross(CameraForwardWorld, CameraRightWorld);

    float3 vertexPosWorldSpace = perModelData.positionWorldSpace 
    + (CameraRightWorld * VAO.inPosition.x * BillboardScale) 
    + (CameraUpWorld * VAO.inPosition.y * BillboardScale);

    VertexOutput out;
    out.vertexData.uv = VAO.inUV;
    
    float4x4 viewProj = mul(globalData.view, globalData.projection);
    out.sv_position = mul(viewProj, float4(vertexPosWorldSpace, 1.0));

    return out;
}
