import Core;
import Material;
import Utils;
import RandomValues;

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
    float depth = _depthTexture.Sample(fsIn.uv).x;
    
    float depthLinear = LinearizeDepth(depth, globalData.viewParams.z, globalData.viewParams.w);
    float3 worldSpacePos = WorldPosFromDepth(depth, fsIn.uv, globalData.invProjection, globalData.inverseView);

    float normalized = saturate((depthLinear- globalData.viewParams.z) / (globalData.viewParams.w - globalData.viewParams.z));
    
    float4 direction = _lightInfo.dirLight.direction;

    float coneAngle = CalculateConeAngle(direction.w, worldSpacePos, direction.xyz);

    //========================
    // initialize randomness
    uint2 pixelCoord = uint2(fsIn.uv * globalData.screenSize);

    uint pixelID = pixelCoord.x + pixelCoord.y * globalData.screenSize.x;
    
    uint randomSeed = tea(pixelID, 0);

    float3 rayDirectionInCone = RandomConeDirection(randomSeed, direction.xyz, coneAngle);

    bool inShadow = false;
    inShadow = IntersectsP(InitRay(worldSpacePos, normalize(rayDirectionInCone), 1000), _tlas);
    
    return float4(float3(inShadow ? 0.5 : 1.0), 1.0);
}
