import ShaderBindingGroups.RTShadow;
import Core;
import Material;
import Utils;
import RandomValues;
import Light;


//=================================
// Set 0

// those two are for shadow calculations 
[[vk::binding(0, 0)]]
public ConstantBuffer<LightInfo> _lightInfo;

[[vk::binding(1, 0)]]
public RaytracingAccelerationStructure _tlas;

[[vk::binding(2, 0)]]
public Sampler2D _positionTexture;

[[vk::binding(3,0)]]
public Sampler2DArray _blueNoiseTextureArray;

[[vk::binding(4,0)]]
public Sampler2D _normalTexture;

[[vk::binding(5, 0)]]
writeonly RWTexture2D<float> visibilityBuffer;

[[vk::binding(6, 0)]]
Sampler2D _motionVector;

[[vk::binding(7, 0)]]
Sampler2D _previousShadow;

struct ShadowMapParameters{
    float sampleLevel;
    bool temporalyAccumulate;
}

[[vk::push_constant]]
ShadowMapParameters _params;

[[shader("compute")]]
[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID : SV_DispatchThreadID){
    float2 dstWriteCoords = float2(threadID.xy);
    float2 dims;
    visibilityBuffer.GetDimensions(dims.x, dims.y);

    if (threadID.x >= dims.x || threadID.y >= dims.y){
        return;
    }

    float2 texCoords = (dstWriteCoords + 0.5) / dims;

    // light is turned off
    if(!(bool)_lightInfo.dirLight.parameters.z){
        visibilityBuffer[threadID.xy].x = 1.0;
        return;
    }

    float3 normal = normalize(_normalTexture.Sample(texCoords)).xyz;

    // get the initial position 
    float3 worldSpacePos = _positionTexture.Sample(texCoords).xyz;
    float3 direction = normalize(_lightInfo.dirLight.direction.xyz);
    float sunRadius = _lightInfo.dirLight.direction.w;

    float blueNoise = _blueNoiseTextureArray.Sample(float3(texCoords,  _params.sampleLevel)).x;

    // TODO: once temporal accumulation is working this will create uniue sample each pixel 
    //blueNoise = fract(blueNoise + goldenRatioConjugate * float(globalData.currentFrame) );

    blueNoise = fract(blueNoise);

    // rotate the blue noise sample 
    float theta = blueNoise * 2.0 * float.getPi();
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

    float nShadowRays = _lightInfo.dirLight.parameters.x;
    float shadowFactor = 0.0;

    float2 diskPoint;

    float3 lightTangent = normalize(cross(direction.xyz , float3(0.0, 1.0, 0.0)));
    float3 lightBitangent = normalize(cross(lightTangent, direction.xyz));


    // calculate shadow bias 
    float shadowBias = 0.1;

    // calculate the pixel ID and rangom state to later use for sampling hte shadow rays 
    uint pixelID = threadID.x + threadID.y * (uint)dims.x;

    for(int i = 0; i < nShadowRays; i++){
        float2 samplePos = BlueNoiseInDisk[i];

        float tHit = 0;

        // rotate the disk randomly, based on the blue noise texture

        diskPoint.x = samplePos.x * cosTheta - samplePos.y * sinTheta;
        diskPoint.y = samplePos.x * sinTheta + samplePos.y * cosTheta;
        
        // scale sampled disk by the radius of the sun making pneumbras 
        diskPoint *= sunRadius;

        // calculate the shadow ray 
        float3 shadowRayDir = normalize(direction.xyz + (diskPoint.x * lightTangent + diskPoint.y * lightBitangent));

        bool inShadow = false;
    
        inShadow = IntersectsP(InitRay(worldSpacePos + normalize(normal) * shadowBias, shadowRayDir, 3000), _tlas, tHit);           
        shadowFactor = lerp(shadowFactor, (inShadow ? 0.0 : 1.0), 1.0/float(i+1)) ;
    }

    if(_params.temporalyAccumulate){
        float alpha = 0.4;
        float2 motion = _motionVector.Sample(texCoords).xy;
        float2 previousTexCoords = texCoords - motion;
        float shadowLast = _previousShadow.Sample(previousTexCoords).x;

        visibilityBuffer[threadID.xy] = alpha * shadowFactor + (1.0f - alpha) * shadowLast;
        return;
    }else{
        visibilityBuffer[threadID.xy] = shadowFactor;
    }


}




