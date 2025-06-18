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
    float3 worldSpacePos = WorldPosFromDepth(depth, fsIn.uv, globalData.invProjection, globalData.inverseView);
    
    float4 direction = _lightInfo.dirLight.direction;

    // blue noise is used to rate the disk of sample positions to get how much it should rotate 
    float blueNoise = _blueNoiseTextureArray.Sample(fsIn.uv).x; // float3 for sample since .z is array level
    //blueNoise = fract(blueNoise + goldenRatioConjugate * float(globalData.currentFrame) );

    blueNoise = fract(blueNoise);

    float theta = blueNoise * 2.0 * float.getPi();
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);


    float raysPerPixel = 10;
    float shadowFactor = 0.0;

    float2 diskPoint;

    float3 lightTangent = normalize(cross(direction.xyz , float3(0.0, 1.0, 0.0)));
    float3 lightBitangent = normalize(cross(lightTangent, direction.xyz));
    
    for(int i = 0; i < _lightInfo.dirLight.shadowRays; ++i){
        float2 samplePos = BlueNoiseInDisk[i];

        // rotate the disk randomly, based on the blue noise texture
        diskPoint.x = samplePos.x * cosTheta - samplePos.y * sinTheta;
        diskPoint.y = samplePos.x * sinTheta + samplePos.y * cosTheta;

        diskPoint *= direction.w;

        float3 shadowRayDir = normalize(direction.xyz + diskPoint.x * lightTangent + diskPoint.y * lightBitangent);

        bool inShadow = false;

        float3 dx = ddx(worldSpacePos);
        float3 dy = ddy(worldSpacePos);
        float3 normal = normalize(cross(dx, dy));
    
        inShadow = IntersectsP(InitRay(worldSpacePos + normal * _lightInfo.dirLight.shadowBias, shadowRayDir, 1000), _tlas);        

        shadowFactor = lerp(shadowFactor, (inShadow ? 0.0 : 1.0), 1.0/float(i+1)) ;
    }


    return float4(float3(shadowFactor), 1.0);
}
