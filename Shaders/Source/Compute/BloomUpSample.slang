struct BloomUpSampleParams{
    float filterRadius;
    float4 src_xy_dst_xy;
}


[[vk::binding(0,0)]]
Sampler2D uabSrcTexture; // the downsampled texture

[[vk::binding(1,0)]]
RWTexture2D uabDstImage; // the up-sampled texture where we up sample + apply blur 

[[vk::push_constant]]
BloomUpSampleParams _bloomUpSampleParams;

[[shader("compute")]]
[numthreads(8, 8, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    int2 dstCoord = threadId.xy;

    float x = _bloomUpSampleParams.filterRadius;
    float y = _bloomUpSampleParams.filterRadius;

    float2 srcRes = _bloomUpSampleParams.src_xy_dst_xy.xy;
    float2 dstRes = _bloomUpSampleParams.src_xy_dst_xy.zw;

    float2 dstUv = (dstCoord + 0.5) / dstRes;
    float2 srcUv = dstUv * srcRes;

    float2 texelSize = 1.0/srcRes;
    float2 texCoords = srcUv * texelSize;

    // Take 9 samples around current texel:w
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = uabSrcTexture.Sample(float2(texCoords.x - x, texCoords.y + y)).rgb;
    float3 b = uabSrcTexture.Sample(float2(texCoords.x,     texCoords.y + y)).rgb;
    float3 c = uabSrcTexture.Sample(float2(texCoords.x + x, texCoords.y + y)).rgb;

    float3 d = uabSrcTexture.Sample(float2(texCoords.x - x, texCoords.y)).rgb;
    float3 e = uabSrcTexture.Sample(float2(texCoords.x,     texCoords.y)).rgb;
    float3 f = uabSrcTexture.Sample(float2(texCoords.x + x, texCoords.y)).rgb;

    float3 g = uabSrcTexture.Sample(float2(texCoords.x - x, texCoords.y - y)).rgb;
    float3 h = uabSrcTexture.Sample(float2(texCoords.x,     texCoords.y - y)).rgb;
    float3 i = uabSrcTexture.Sample(float2(texCoords.x + x, texCoords.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    float3 upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;

    // X` = X + blur(X-1, b4)
    // aditive blending, add whatever was in this mip layer with blured result of the lower mip layer 
    uabDstImage[uint2(dstCoord )].xyz += upsample;
}