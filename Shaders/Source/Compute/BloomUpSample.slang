struct BloomUpSampleParams{
    float filterRadius;
}


[[vk::binding(0,0)]]
Sampler2D SrcImage; // the downsampled texture

[[vk::binding(1,0)]]
RWTexture2D DstImage; // the up-sampled texture where we up sample + apply blur 


[[vk::push_constant]]
BloomUpSampleParams _bloomUpSampleParams;

[[shader("compute")]]
[numthreads(8, 8, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    float x = _bloomUpSampleParams.filterRadius;
    float y = _bloomUpSampleParams.filterRadius;

    float2 srcTexCoords = 

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = srcTexture.Sample(float2(texCoord.x - x, texCoord.y + y)).rgb;
    float3 b = srcTexture.Sample(float2(texCoord.x,     texCoord.y + y)).rgb;
    float3 c = srcTexture.Sample(float2(texCoord.x + x, texCoord.y + y)).rgb;

    float3 d = srcTexture.Sample(float2(texCoord.x - x, texCoord.y)).rgb;
    float3 e = srcTexture.Sample(float2(texCoord.x,     texCoord.y)).rgb;
    float3 f = srcTexture.Sample(float2(texCoord.x + x, texCoord.y)).rgb;

    float3 g = srcTexture.Sample(float2(texCoord.x - x, texCoord.y - y)).rgb;
    float3 h = srcTexture.Sample(float2(texCoord.x,     texCoord.y - y)).rgb;
    float3 i = srcTexture.Sample(float2(texCoord.x + x, texCoord.y - y)).rgb;

}