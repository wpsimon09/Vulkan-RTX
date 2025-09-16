import Core;

struct BloomDownSampleParams{
    float4 src_dst_resolution;
}

[[vk::push_constant]]
ConstantBuffer<BloomDownSampleParams> _bloomDownsampleParams;

[[vk::binding(0,0)]]
Sampler2D uabSrcTexture; // rendered texture with shadows and lighting 

[[vk::binding(1,0)]]
writeonly RWTexture2D uabDownSampleResult; 

[[shader("compute")]]
[numthreads(8, 8, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    uint2 dstCoord = threadId.xy;
    
    // Get resolutions from push constants
    float2 srcResolution = float2(_bloomDownsampleParams.src_dst_resolution.x, _bloomDownsampleParams.src_dst_resolution.y);
    float2 dstResolution = float2(_bloomDownsampleParams.src_dst_resolution.z, _bloomDownsampleParams.src_dst_resolution.w);
    
    // Calculate source texture coordinates from destination coordinates
    float2 srcTexCoord = (dstCoord + 0.5) / dstResolution; // Normalized coords for destination
    srcTexCoord = srcTexCoord * (dstResolution / srcResolution); // Scale to source texture space

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
    float3 a = uabSrcTexture.Sample( float2(srcTexCoord.x - 2*x, srcTexCoord.y + 2*y)).rgb;
    float3 b = uabSrcTexture.Sample( float2(srcTexCoord.x,       srcTexCoord.y + 2*y)).rgb;
    float3 c = uabSrcTexture.Sample( float2(srcTexCoord.x + 2*x, srcTexCoord.y + 2*y)).rgb;

    float3 d = uabSrcTexture.Sample( float2(srcTexCoord.x - 2*x, srcTexCoord.y)).rgb;
    float3 e = uabSrcTexture.Sample( float2(srcTexCoord.x,       srcTexCoord.y)).rgb;
    float3 f = uabSrcTexture.Sample( float2(srcTexCoord.x + 2*x, srcTexCoord.y)).rgb;

    float3 g = uabSrcTexture.Sample( float2(srcTexCoord.x - 2*x, srcTexCoord.y - 2*y)).rgb;
    float3 h = uabSrcTexture.Sample( float2(srcTexCoord.x,       srcTexCoord.y - 2*y)).rgb;
    float3 i = uabSrcTexture.Sample( float2(srcTexCoord.x + 2*x, srcTexCoord.y - 2*y)).rgb;

    float3 j = uabSrcTexture.Sample( float2(srcTexCoord.x - x, srcTexCoord.y + y)).rgb;
    float3 k = uabSrcTexture.Sample( float2(srcTexCoord.x + x, srcTexCoord.y + y)).rgb;
    float3 l = uabSrcTexture.Sample( float2(srcTexCoord.x - x, srcTexCoord.y - y)).rgb;
    float3 m = uabSrcTexture.Sample( float2(srcTexCoord.x + x, srcTexCoord.y - y)).rgb;

    float3 downSample = e* 0.125;
    downSample += (a+c+g+i)*0.03125;
    downSample += (b+d+f+h)*0.0625;
    downSample += (j+k+l+m)*0.125;

    uabDownSampleResult[int2(dstCoord.xy)].xyz = downSample; 

}

