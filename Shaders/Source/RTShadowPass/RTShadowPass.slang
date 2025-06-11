import Core;
import Material;
import Utils;
#include "ShaderBindingGroups/RTShadow.slang"

struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
};

[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    // draw indexed with 3 vertices, requires no VAO binding
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) :SV_Target {
    float depth = _depthTexture.Sample<Texture2D, false, true, >(fsIn.uv).x;
    float depthLinear = LinearizeDepth(depth, globalData.viewParams.z, globalData.viewParams.w);
    float3 worldSpacePos = WorldPosFromDepth(depth, fsIn.uv, globalData.invProjection, globalData.inverseView);

    float normalized = saturate((depthLinear- globalData.viewParams.z) / (globalData.viewParams.w - globalData.viewParams.z));
    
    bool inShadow = false;
    inShadow = IntersectsP(InitRay(worldSpacePos, normalize(_lightInfo.dirLight.direction.xyz), 1000), _tlas);
    
    return float4(normalized.xxx, 1.0);
    //return float4(fsIn.uv, 1.0, 1.0);

}
