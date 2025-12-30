import Core;
import ColourSpaceConversions;

struct BloomDownSampleParams{
    float4 src_dst_resolution;
    int srcImage;
    int dstImage;
}

[[vk::push_constant]]
ConstantBuffer<BloomDownSampleParams> _bloomDownsampleParams;

[[vk::binding(0,0)]]
Sampler2D srcTextures[]; // rendered texture with shadows and lighting 

[[vk::binding(1,0)]]
writeonly RWTexture2D _downSampleResults[]; 

float KarisAverage(float3 colour){
    float luma = RGBToLuminance(colour) * 0.25;
    return 1.0f / (1.0f + luma);
}

[[shader("compute")]]
[numthreads(8, 8, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    uint2 dstCoord = threadId.xy;
    
    // Get resolutions from push constants
    float2 srcResolution = float2(_bloomDownsampleParams.src_dst_resolution.x, _bloomDownsampleParams.src_dst_resolution.y);
    float2 dstResolution = float2(_bloomDownsampleParams.src_dst_resolution.z, _bloomDownsampleParams.src_dst_resolution.w);

     if(threadId.x >= dstResolution.x || threadId.y >= dstResolution.y){
        return;
    }
    
    
    // since i am using sampler the tex coordinates have to be in 0 - 1 range which is achieved by deviding by the src resolution
    float2 srcTexCoord = (float2(threadId.xy) + 0.5) / dstResolution;

    float2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x - 2*x, srcTexCoord.y + 2*y)).rgb;
    float3 b = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x,       srcTexCoord.y + 2*y)).rgb;
    float3 c = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x + 2*x, srcTexCoord.y + 2*y)).rgb;

    float3 d = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x - 2*x, srcTexCoord.y)).rgb;
    float3 e = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x,       srcTexCoord.y)).rgb;
    float3 f = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x + 2*x, srcTexCoord.y)).rgb;

    float3 g = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x - 2*x, srcTexCoord.y - 2*y)).rgb;
    float3 h = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x,       srcTexCoord.y - 2*y)).rgb;
    float3 i = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x + 2*x, srcTexCoord.y - 2*y)).rgb;

    float3 j = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x - x, srcTexCoord.y + y)).rgb;
    float3 k = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x + x, srcTexCoord.y + y)).rgb;
    float3 l = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x - x, srcTexCoord.y - y)).rgb;
    float3 m = srcTextures[_bloomDownsampleParams.srcImage].Sample( float2(srcTexCoord.x + x, srcTexCoord.y - y)).rgb;

    float3 groups[5];
    if(_bloomDownsampleParams.srcImage == 0){
        groups[0] = (a+b+d+e) * (0.125f/4.0f);
        groups[1] = (b+c+e+f) * (0.125f/4.0f);
        groups[2] = (d+e+g+h) * (0.125f/4.0f);
        groups[3] = (e+f+h+i) * (0.125f/4.0f);
        groups[4] = (j+k+l+m) * (0.5f/4.0f);
        groups[0] *= KarisAverage(groups[0]);
        groups[1] *= KarisAverage(groups[1]);
        groups[2] *= KarisAverage(groups[2]);
        groups[3] *= KarisAverage(groups[3]);
        groups[4] *= KarisAverage(groups[4]);
        _downSampleResults[_bloomDownsampleParams.dstImage][dstCoord.xy].xyz = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
        return;
    }
    float3 downSample = e* 0.125;
    downSample += (a+c+g+i)*0.03125;
    downSample += (b+d+f+h)*0.0625;
    downSample += (j+k+l+m)*0.125;


    _downSampleResults[_bloomDownsampleParams.dstImage][dstCoord.xy].xyz = max(downSample, 0.0001f); 

}

