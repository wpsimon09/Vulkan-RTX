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


void compute_default_basis(const float3 normal, out float3 x, out float3 y, out float3 z)
{
  // ZAP's default coordinate system for compatibility
  z              = normal;
  const float yz = -z.y * z.z;
  y = normalize(((abs(z.z) > 0.99999f) ? float3(-z.x * z.y, 1.0f - z.y * z.y, yz) : float3(-z.x * z.z, yz, 1.0f - z.z * z.z)));

  x = cross(y, z);
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

    if(globalData.aoOcclusion){
        float3 x, y, z;
        compute_default_basis(normal, x, y, z);
        for(int i = 0; i < 4; ++i){

            float r1 = radinv2(rng);

            float r2 = radinv_fl(rng, 3);

            float sq = sqrt(1.0 - r2);

            float3 direction = float3(cos(2 * PI * r1) * sq, sin(2 * PI * r1) * sq, sqrt(r2));
            direction      = direction.x * x + direction.y * y + direction.z * z;

            rng++;

            float tHit = 2000;
            bool occluded = IntersectsP(InitRay(worldSpacePos + normalize(normal) * 0.3, direction, tHit),_tlas, tHit);
            
            aoFactor += occluded ? 1.0 : 0.0; 
        }

        aoFactor = 1.0 - (aoFactor / 4);
        aoFactor = clamp(aoFactor, 0, 1);
    }


    //return float4(float3(aoFactor), 1.0);


    return float4(shadowFactor, globalData.aoOcclusion ? aoFactor : 1.0 , 0.0,  1.0);
}