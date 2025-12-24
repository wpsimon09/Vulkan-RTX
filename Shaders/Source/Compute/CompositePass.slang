import Core;

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

ConstantBuffer<GlobalData2> _globalData;

[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID : SV_DispatchThreadID){

    var parsedGlobalData = _globalData.Parse();

    uint2 dims;
    _finalImage.GetDimensions(dims.x, dims.y);

    float2 texCoords = (threadID.xy + 0.5) / dims;
    
    float3 col = _renderedImage.Sample(texCoords).xyz;
    
    if(_depthMap.Sample(texCoords).x > 0.999){
      _finalImage[threadID.xy] = float4(col,1.0); 
      return;
    }
    
    float shadowFactor = _dirLightShadowMap.Sample(texCoords).x;
    
    float occlusionStrenth = shadowFactor;
        
    col = lerp(_albedoMap.Sample(texCoords).xyz * 0.5, col, shadowFactor);
    
    if(parsedGlobalData.aoOcclusion){
        float aoFactor = _aoMap.Sample(texCoords).x;
        col *= aoFactor;
    }

    if(parsedGlobalData.useReflection){
        float3 reflectionValue = _reflectionMap.Sample(texCoords).xyz;
        float roughness = _armMap.Sample(texCoords).x;
        col = lerp(reflectionValue, col ,clamp(roughness, 0.3, 1.0));
    }

    _finalImage[threadID.xy] = float4(col,1.0f);
}