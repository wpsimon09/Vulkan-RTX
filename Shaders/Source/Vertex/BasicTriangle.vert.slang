import Utils;
import Material;
import Core;
#include "ShaderBindingGroups/ForwardLit.slang"

struct VertexInput {
    float3 inPosition :POSITION ;
    float3 inNormal : NORMAL;
    float2 inUV : UV;
    float4 inTangent : TANGENT;
};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;
    float4 sv_position : SV_Position;
}

    
    [[vk::push_constant]]
    PerObjectPushConstatn perObjectPC;

    struct FragmentShaderIn {
        float3 normal : OUT_NORMAL;
        float3 cameraPos : OUT_CAM_POS;
        float3 fragPos : OUT_FRAG_POS;
        float2 uv : OUT_UV;
        int materialIndex : OUT_MATERIAL;
        float2 screenSpaceUV: OUT_SCREEN_UV;
        float3x3 TBN : OUT_TBN;
    };


    [shader("vertex")]
    VertexOutput vertexMain(VertexInput VAO) // VAO input{
    {
        // sotre for later
        float3 vertexPos = VAO.inPosition;
        float3 normal = VAO.inNormal;
        float2 uv = VAO.inUV;


        float4x4 normalMatrix = transpose(inverse(perObjectPC.modelMatrix));
        float3 FragmentWorldPos =mul( perObjectPC.modelMatrix,  float4(vertexPos, 1.0)).xyz;
        float3 NormalWorldPos = mul(normalMatrix, float4(normal.xyz, 1.0)).xyz;//mul( float4(normal, 1.0) , perObjectData[perObjectPC.perObjectDataIndex].normalMatrix).xyz;
        
        float4x4 viewProj = mul(globalData.projection, globalData.view);
        
        VertexOutput out;
        out.sv_position = mul(viewProj, float4(FragmentWorldPos, 1.0));
        
        out.vertexData.TBN = CalculateTBN(VAO.inNormal, VAO.inTangent, float3x3(normalMatrix));
        out.vertexData.cameraPos = globalData.cameraPostiion.xyz;
        out.vertexData.fragPos = FragmentWorldPos;
        out.vertexData.normal = NormalWorldPos;
        out.vertexData.uv = uv;
        out.vertexData.materialIndex = perObjectData[perObjectPC.perObjectDataIndex.x].indexes.x;
        out.vertexData.screenSpaceUV = globalData.screenSize;

        return out;
    }
