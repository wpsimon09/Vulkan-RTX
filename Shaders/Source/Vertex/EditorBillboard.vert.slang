import Utils;
import Core;
import Material;
struct VertexInput {
    float3 inPosition :POSITION ;
    float2 inUV : UV;
};

struct FragmentShaderIn {
    float2 uv : OUT_UV;
    Material mat : OUT_MAT;
};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;

    float4 sv_position : SV_Position;
}


[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> globalData;

[[vk::binding(1, 0)]]
StructuredBuffer<Material> materials;

[[vk::binding(0, 1)]]
ConstantBuffer<PermodelData> perModelData ;

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    const float BillboardScale = 30.f;

    // sotre for later    
    float3 CameraForwardWorld = normalize(globalData.cameraPostiion.xyz - perModelData.positionWorldSpace.xyz);
    float3 CameraUpWorld = { 0, 1, 0};
    float3 CameraRightWorld = normalize(cross(CameraUpWorld, CameraForwardWorld));
    float3 Up = cross(CameraForwardWorld, CameraRightWorld);

    float3 vertexPosWorldSpace = perModelData.positionWorldSpace.xyz 
    + (CameraRightWorld * VAO.inPosition.x * BillboardScale) 
    + (CameraUpWorld * VAO.inPosition.y * BillboardScale);

    VertexOutput out;
    out.vertexData.uv = VAO.inUV;
    out.vertexData.mat = materials[perModelData.materialIndex];
    
    float4x4 viewProj = mul(globalData.view, globalData.projection);
    out.sv_position = mul(viewProj, float4(vertexPosWorldSpace, 1.0));

    return out;
}
