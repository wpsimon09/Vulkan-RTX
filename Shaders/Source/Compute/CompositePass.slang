import Core;
import Utils;
import BxDF;

[[vk::binding(0,0)]]
RWTexture2D _finalImage;

[[vk::binding(1,0)]]
Sampler2D _dirLightShadowMap;

[[vk::binding(2,0)]]
Sampler2D _aoMap;

[[vk::binding(3,0)]]
Sampler2D _renderedImage;

[[vk::binding(4,0)]]
Sampler2D _albedoMap;

[[vk::binding(5,0)]]
Sampler2D _depthMap;

[[vk::binding(6,0)]]
Sampler2D _armMap;

[[vk::binding(7,0)]]
Sampler2D _reflectionMap;

[[vk::binding(8, 0)]]
Sampler2D _normalMap;

[[vk::binding(9, 0)]]
ConstantBuffer<GlobalData2> _globalData;

[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID : SV_DispatchThreadID){

    var parsedGlobalData = _globalData.Parse();

    uint2 dims;
    _finalImage.GetDimensions(dims.x, dims.y);

    float2 texCoords = (threadID.xy + 0.5) / dims;
    
    float3 col = _renderedImage.Sample(texCoords).xyz;
    float3 N = normalize(_normalMap.Sample(texCoords).xyz);
    
    float depth = _depthMap.Sample(texCoords).x;
    if(depth > 0.999){
        _finalImage[threadID.xy] = float4(col,1.0); 
        return;
    }

    float3 position = WorldPosFromDepth(depth, texCoords,_globalData.inverseProj, _globalData.inverseView);
    
    float shadowFactor = _dirLightShadowMap.Sample(texCoords).x;
    
    float occlusionStrenth = shadowFactor;
        
    float3 albedo = _albedoMap.Sample(texCoords).xyz;
    col = lerp(albedo * 0.5, col, shadowFactor);
    
    if(parsedGlobalData.aoOcclusion){
        float aoFactor = _aoMap.Sample(texCoords).x;
        col *= aoFactor;
    }

    if(parsedGlobalData.useReflection){
        float3 reflectionValue = _reflectionMap.Sample(texCoords).xyz;
        float2 roughnessMetallnes = _armMap.Sample(texCoords).xy;
        float3 F0 = lerp(0.04,albedo,roughnessMetallnes.y );
        float3 V = normalize(_globalData.cameraPosition.xyz - position );
        
        float cosTheta = max(dot(N, V ), 0.0);
        
        float3 F = FresnelSchlick(cosTheta, F0);

        float3 specualarWeight = (1.0 - roughnessMetallnes.x);

        float3 specular = reflectionValue * ( specualarWeight);

        col += specular;
    }

    _finalImage[threadID.xy] = float4(col,1.0f);
}