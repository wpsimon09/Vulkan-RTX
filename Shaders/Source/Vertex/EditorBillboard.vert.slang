import Utils;
#include "ShaderBindingGroups/ForwardUnlit.slang"

struct VertexInput {
    float3 inPosition :POSITION ;
    float2 inUV : UV;
};

struct FragmentShaderIn {
    float2 uv : OUT_UV;
    int mat : OUT_MAT;
};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;

    float4 sv_position : SV_Position;
}

[[vk_push_constant]]
PerObjectPushConstatn perObjectPC;


[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    const float BillboardScale = 30.f;

    // sotre for later
    float3 CameraForwardWorld = normalize(globalData.cameraPostiion.xyz - perModelData[perObjectPC.perObjectDataIndex.x].positionWorldSpace.xyz);
    float3 CameraUpWorld = { 0, 1, 0};
    float3 CameraRightWorld = normalize(cross(CameraUpWorld, CameraForwardWorld));
    float3 Up = cross(CameraForwardWorld, CameraRightWorld);

    float3 vertexPosWorldSpace = perModelData[perObjectPC.perObjectDataIndex.x].positionWorldSpace.xyz 
    + (CameraRightWorld * VAO.inPosition.x * BillboardScale) 
    + (CameraUpWorld * VAO.inPosition.y * BillboardScale);

    VertexOutput out;
    out.vertexData.uv = VAO.inUV;
    out.vertexData.mat = perModelData[perObjectPC.perObjectDataIndex.x].indexes.x;
    
    float4x4 viewProj = mul(globalData.view, globalData.projection);
    out.sv_position = mul(viewProj, float4(vertexPosWorldSpace, 1.0));

    return out;
}
