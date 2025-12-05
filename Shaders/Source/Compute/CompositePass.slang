import Core;

RWTexture2D _finalImage;

Sampler2D _dirLightShadowMap;
Sampler2D _aoMap;
Sampler2D _renderedImage;

ConstantBuffer<GlobalData> _globalData;

[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID : SV_DispatchThreadID){
    uint2 dims;
    _finalImage.GetDimensions(dims.x, dims.y);

    float2 texCoords = (float2(threadID.xy) + 0.5) / dims;
    
    float shadowFactor = _dirLightShadowMap.Sample(texCoords).x;

    float occlusionStrenth = shadowFactor;

    if(_globalData.aoOcclusion){
        float aoFactor = _aoMap.Sample(texCoords).x;
        //occlusionStrenth *= aoFactor;
    }

    float3 col = _renderedImage.Sample(texCoords).xyz;

    _finalImage[threadID.xy].xyz = col * occlusionStrenth;
}