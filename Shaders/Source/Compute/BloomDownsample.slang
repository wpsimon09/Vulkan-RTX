import Core;

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

[[shader("compute")]]
[numthreads(8, 8, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    uint2 dstCoord = threadId.xy;
    
    
    // Get resolutions from push constants
    float2 srcResolution = float2(_bloomDownsampleParams.src_dst_resolution.x, _bloomDownsampleParams.src_dst_resolution.y);
    float2 dstResolution = float2(_bloomDownsampleParams.src_dst_resolution.z, _bloomDownsampleParams.src_dst_resolution.w);
    
    // since i am using sampler the tex coordinates have to be in 0 - 1 range which is achieved by deviding by the src resolution
    float2 srcTexCoord = (float2(threadId.xy) + 0.5) / srcResolution;

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

    float3 downSample = e* 0.125;
    downSample += (a+c+g+i)*0.03125;
    downSample += (b+d+f+h)*0.0625;
    downSample += (j+k+l+m)*0.125;


    _downSampleResults[_bloomDownsampleParams.dstImage][dstCoord.xy].xyz = downSample; 

}

