struct BloomUpSampleParams{
    float4 src_xy_dst_xy;
    float filterRadius;
    float srcImage;
    float dstImage;
    float padding;
}


[[vk::binding(0,0)]]
Sampler2D SrcTextures[]; // the downsampled texture

[[vk::binding(1,0)]]
RWTexture2D DstImage[]; // the up-sampled texture where we up sample + apply blur 

[[vk::push_constant]]
BloomUpSampleParams _bloomUpSampleParams;

[[shader("compute")]]
[numthreads(16, 16, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    int2 dstCoord = threadId.xy;
    
    float2 srcRes = _bloomUpSampleParams.src_xy_dst_xy.xy;
    float2 dstRes = _bloomUpSampleParams.src_xy_dst_xy.zw;
    if(threadId.x >= dstRes.x || threadId.y >= dstRes.y){
        DstImage[(int)_bloomUpSampleParams.dstImage][uint2(dstCoord )].xyz = float3(0.0);
        return;
    }
    

    float2 texelSize = 1.0 / srcRes;
    // filter is in textel size
    float x = _bloomUpSampleParams.filterRadius * texelSize.x;
    float y = _bloomUpSampleParams.filterRadius * texelSize.y;

    float2 dstUv = (dstCoord + 0.5) / dstRes;
    float2 srcUv = dstUv;

    float2 texCoords = srcUv;

    // Take 9 samples around current texel:w
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x - x, texCoords.y + y)).rgb;
    float3 b = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x,     texCoords.y + y)).rgb;
    float3 c = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x + x, texCoords.y + y)).rgb;

    float3 d = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x - x, texCoords.y)).rgb;
    float3 e = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x,     texCoords.y)).rgb;
    float3 f = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x + x, texCoords.y)).rgb;

    float3 g = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x - x, texCoords.y - y)).rgb;
    float3 h = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x,     texCoords.y - y)).rgb;
    float3 i = SrcTextures[(int)_bloomUpSampleParams.srcImage].Sample(float2(texCoords.x + x, texCoords.y - y)).rgb;

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
    if (_bloomUpSampleParams.dstImage == 0){
        DstImage[(int)_bloomUpSampleParams.dstImage][uint2(dstCoord )].xyz = upsample;
    }else{
        DstImage[(int)_bloomUpSampleParams.dstImage][uint2(dstCoord )].xyz += upsample;
    }
}