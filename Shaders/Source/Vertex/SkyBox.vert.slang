import Core;
#include "ShaderBindingGroups/SkyBox.slang"

struct VertexInput {
    float3 inPosition :POSITION ;
};

struct FragmentShaderIn {
    float3 localPos : OUT_LOCAL_POS;
    float2 uv : OUT_UV;
};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;
    float4 sv_position : SV_Position;
}


[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    float3 vertexPos = VAO.inPosition;

    VertexOutput out;
    out.vertexData.localPos = vertexPos;
    
    column_major float4x4 rotView;
    rotView[0] = float4(globalData.view[0].xyz, 0.0);
    rotView[1] = float4(globalData.view[1].xyz, 0.0);
    rotView[2] = float4(globalData.view[2].xyz, 0.0);
    rotView[3] = float4(0.0, 0.0, 0.0, 1.0);
    
    column_major float4x4 viewProj = mul(globalData.projection, rotView);
    float4 pos = mul(viewProj, float4(VAO.inPosition, 1.0));
    out.sv_position = pos.xyww;

    return out;
}   
