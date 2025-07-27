import Core;
import Material;
import Utils;
#include "ShaderBindingGroups/ForwardUnlitNoMaterials.slang"

struct VertexInput {
    float3 inPosition :POSITION ;
    float3 inNormal: NORMAL;
};

struct VertexOutput {
    float4 sv_position : SV_Position;
    float3 worldPos : WORLD_POS;
    float3 worldNormal : WORLD_N;
}

[[vk::push_constant]]
PerObjectPushConstatn perObjectPC;

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO)
{
    // sotre for later
    float3 vertexPos = VAO.inPosition;

    float3 FragmentWorldPos = mul(perObjectPC.modelMatrix, float4(vertexPos, 1.0)).xyz;

    VertexOutput out;
    
    float4x4 viewProj = mul(globalData.projection, globalData.view);
    out.sv_position = mul(viewProj, float4(FragmentWorldPos, 1.0));
    out.worldPos = FragmentWorldPos;

    float4x4 normalMatrix = transpose(inverse(perObjectPC.modelMatrix));
    float3 NormalWorldPos = mul(normalMatrix, float4(VAO.inNormal.xyz, 0.0)).xyz;
    out.worldNormal = NormalWorldPos;

    return out;
}

struct FragmentOut{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
}

[shader("fragment")]
FragmentOut fragmentMain(VertexOutput fsIn) {
    FragmentOut out;

    out.position = float4(fsIn.worldNormal, 1.0);
    out.normal = float4(fsIn.worldNormal, 1.0);

    return out;
}
