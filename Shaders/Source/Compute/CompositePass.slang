RWTexture2D _finalImage;

Sampler2D _dirLightShadowMap;
Sampler2D _aoMap;
Sampler2D _renderedImage;

[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID : SV_DispatchThreadID){
    uint2 dims;
    _finalImage.GetDimensions(dims.x, dims.y);

    float2 texCoords = (float2(threadID.xy) + 0.5) / dims;
    
    float shadowFactor = _dirLightShadowMap.Sample(texCoords).y;
    float aoFactor = _aoMap.Sample(texCoords).x;

    float occlusionStrenth = aoFactor * shadowFactor;

    float3 col = _renderedImage.Sample(texCoords.x).xyz;

    _finalImage[threadID.xy].xyz = col * occlusionStrenth;

}