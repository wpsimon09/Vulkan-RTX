import Core;

RWTexture2D _finalImage;

Sampler2D _dirLightShadowMap;
Sampler2D _aoMap;
Sampler2D _renderedImage;
Sampler2D _albedoMap;
Sampler2D _depthMap;

ConstantBuffer<GlobalData> _globalData;

[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID : SV_DispatchThreadID){
    uint2 dims;
    _finalImage.GetDimensions(dims.x, dims.y);

    float2 texCoords = (float2(threadID.xy) + 0.5) / dims;
    
    float3 col = _renderedImage.Sample(texCoords).xyz;
    
    if(_depthMap.Sample(texCoords).x == 1.0){
        _finalImage[threadID.xy] = float4(col,1.0); 
        return;
    }
    
    float shadowFactor = _dirLightShadowMap.Sample(texCoords).x;
    
    float occlusionStrenth = shadowFactor;
    
    if(_globalData.aoOcclusion){
        float aoFactor = _aoMap.Sample(texCoords).x;
        occlusionStrenth *= aoFactor;
    }
    

    col = lerp(_albedoMap.Sample(texCoords).xyz * 0.4, col, occlusionStrenth);

    _finalImage[threadID.xy].xyz = col * occlusionStrenth;
}