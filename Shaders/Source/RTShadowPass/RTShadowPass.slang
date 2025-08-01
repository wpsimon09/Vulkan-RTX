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

    if(!(bool)_lightInfo.dirLight.parameters.z){
        return float4(1.0);
    }

    float3 worldSpacePos = _positionTexture.Sample(fsIn.uv).xyz;
    
    // get neighbouring positions in world space 
    float3 px = _positionTexture.Sample(fsIn.uv, int2(1, 0)).xyz;
    float3 py = _positionTexture.Sample(fsIn.uv, int2(0, 1)).xyz;
    
    // calcualte tangent of the world plane 
    float3 Tx = normalize(px - worldSpacePos);
    float3 Ty = normalize(py - worldSpacePos);

    //approximate normal for shadow bias
    float3 N_approx = cross(Tx, Ty);

    float4 direction = normalize(_lightInfo.dirLight.direction);

    // blue noise is used to rate the disk of sample positions to get how much it should rotate 
    float blueNoise = _blueNoiseTextureArray.Sample(fsIn.uv).x; // float3 for sample since .z is array level
    //blueNoise = fract(blueNoise + goldenRatioConjugate * float(globalData.currentFrame) );

    blueNoise = fract(blueNoise);

    float theta = blueNoise * 2.0 * float.getPi();
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);


    float raysPerPixel = 10;
    float shadowFactor = 0.0;
    float aoFactor = 0.0;

    float2 diskPoint;

    float3 lightTangent = normalize(cross(direction.xyz , float3(0.0, 1.0, 0.0)));
    float3 lightBitangent = normalize(cross(lightTangent, direction.xyz));

    float nShadowRays = _lightInfo.dirLight.parameters.x;
    float shadowBias = _lightInfo.dirLight.parameters.y;

    uint2 pixelCoord = uint2(fsIn.uv * globalData.screenSize);

    uint pixelID = fsIn.uv.x + pixelCoord.y * globalData.screenSize.x;

    uint rng = tea(pixelID, 0);

    float disatnceTraveled = 0;
    
    float3 normal = _normalTexture.Sample(fsIn.uv).xyz;
    
    for(int i = 0; i < 3; ++i){
        float2 samplePos = BlueNoiseInDisk[i];

        float tHit = 0;

        // rotate the disk randomly, based on the blue noise texture
        diskPoint.x = samplePos.x * cosTheta - samplePos.y * sinTheta;
        diskPoint.y = samplePos.x * sinTheta + samplePos.y * cosTheta;

        // direction.w is sun radius 
        diskPoint *= direction.w;

        float3 shadowRayDir = normalize(direction.xyz + (diskPoint.x * lightTangent + diskPoint.y * lightBitangent));

        bool inShadow = false;
    
        inShadow = IntersectsP(InitRay( worldSpacePos + normalize(normal) * shadowBias, shadowRayDir, 1000), _tlas, tHit);   
    
        disatnceTraveled += tHit;

        
        shadowFactor = lerp(shadowFactor, (inShadow ? 0.0 : 1.0), 1.0/float(i+1)) ;
    }

    for(int i = 0; i < 3; ++i){
        float3 direction = RandomHemisphereDirection(normal, rng);
        float tHit = 4;
        bool occluded = IntersectsP(InitRay(worldSpacePos + normalize(normal) * 0.2, direction, tHit),_tlas, tHit);
        
        aoFactor = lerp(aoFactor, (occluded ? 0.0 : 1.0), 1.0/float(i+1)); 
    }

    return float4(shadowFactor, aoFactor, 0.0,  1.0);

}
