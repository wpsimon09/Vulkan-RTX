import Core;
import Material;

struct VertexInput {
    float3 inPosition :POSITION ;
};

struct VertexOutput {
    float4 sv_position : SV_Position;
}


[[vk::binding(0,0)]]
ConstantBuffer<GlobalData> globalData;


[[vk::binding(0, 1)]]
StructuredBuffer<PermodelData> perModelData ;

[[vk::push_constant]]
uint perObjectIndex;


[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO)
{
    // sotre for later
    float3 vertexPos = VAO.inPosition;

    float3 FragmentWorldPos = mul(perModelData[perObjectIndex].modelMatrix, float4(vertexPos, 1.0)).xyz;

    VertexOutput out;
    
    float4x4 viewProj = mul(globalData.view, globalData.projection);
    out.sv_position = mul(viewProj, float4(FragmentWorldPos, 1.0));

    return out;
}

[shader("fragment")]
void fragmentMain() {

}
