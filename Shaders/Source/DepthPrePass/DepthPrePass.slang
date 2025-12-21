import Core;
import Material;
import Utils;
#include "ShaderBindingGroups/ForwardUnlitNoMaterials.slang"

struct VertexInput {
    float3 inPosition :POSITION ;
    float3 inNormal: NORMAL;
    float2 inUv : UV;
    float4 inTangent : TANGENT;
};

struct VertexOutput {
    float4 sv_position : SV_Position;
    float4 worldPos : WORLD_POS;
    float3 worldNormal : WORLD_N;
    float2 uv : UV;
    float4 worldPosPrevFrame : WORLD_POS_PREV_FRAME;
    float3x3 TBN : TBN;
}

[[vk::push_constant]]
PerObjectPushConstatn perObjectPC;

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO)
{
    
    // sotre for later
    float3 vertexPos = VAO.inPosition;
    
    float3 FragmentWorldPos = mul(perObjectPC.modelMatrix, float4(vertexPos, 1.0)).xyz;
    float3 vertexPosPreFrame = mul(perObjectPC.previousModelMatrix, float4(vertexPos, 1.0)).xyz;
    
    VertexOutput out;
    
    //===============================
    // current world space position
    float4x4 viewProj = mul(globalData.proj, globalData.view);
    out.sv_position = mul(viewProj, float4(FragmentWorldPos, 1.0));
    out.worldPos = float4(FragmentWorldPos, 1.0);

    //=============================
    // previosu world space postion
    float4x4 viewPorjPrevFrame = mul(globalData.projPrevFrame, globalData.viewPrevFrame);
    out.worldPosPrevFrame = mul(viewPorjPrevFrame , float4(vertexPosPreFrame,1.0));

    //=============================
    // Transfer the normal vectors 
    float4x4 normalMatrix = transpose(inverse(perObjectPC.modelMatrix));
    float3 NormalWorldPos = mul(normalMatrix, float4(VAO.inNormal.xyz, 0.0)).xyz;
    out.worldNormal = NormalWorldPos;
    out.uv = VAO.inUv;
    out.TBN = CalculateTBN(VAO.inNormal, VAO.inTangent, (float3x3)normalMatrix);

    return out;
}

struct FragmentOut{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 albedo : SV_Target2;
    float4 arm : SV_Target3;
    float4 motionVector : SV_Target4;
}

[shader("fragment")]
FragmentOut fragmentMain(VertexOutput fsIn) {
    FragmentOut out;

    Material mat = materialValues[perObjectData[perObjectPC.perObjectDataIndex.x].indexes.x];

    PBRMaterial material;
    material.features = mat.Features;
    material.values = mat.Values;

    float4 albedo = float4(0.0);
    if(material.features.hasDiffuseTexture){
        albedo = textures[material.features.albedoTextureIdx].Sample(fsIn.uv);
    }else{
        albedo = material.values.diffuse;
    }
    
    float3 arm;
    if(material.features.hasArmTexture){
        arm.xy = textures[material.features.armTextureIdx].Sample(fsIn.uv).xy;
        arm.z = 1.0;
    }else{
        arm.x = material.values.roughness;
        arm.y = material.values.metalness;
        arm.z = 1.0;
    }
    
    float3 N = fsIn.worldNormal;
    
    if(material.features.hasNormalTexture){
    //    N = SampleNormalMap(textures[material.features.normalTextureIdx], fsIn.uv, fsIn.TBN);
    }

    out.position = float4(fsIn.worldPos.xyz, 1.0);
    out.normal = float4(fsIn.worldNormal, 1.0);
    out.albedo = float4(albedo.xyz,1.0);
    out.arm = float4(arm.xy,0.0,1.0);

    float2 currentPosTextureSpace = (fsIn.worldPos.xy / fsIn.worldPos.w) * 0.5 + 0.5;
    float2 prevPosTextureSpace = (fsIn.worldPosPrevFrame.xy / fsIn.worldPosPrevFrame.w) * 0.5 + 0.5;
    out.motionVector = float4(currentPosTextureSpace - prevPosTextureSpace, 0.0, 1.0);

    return out;
}
