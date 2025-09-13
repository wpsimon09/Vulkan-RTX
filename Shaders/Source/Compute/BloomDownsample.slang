import Core;

struct BloomDownSampleParams{
    float4 src_dst_resolution;
    float currentMip;
}

[[vk::push_constant]]
ConstantBuffer<BloomDownSampleParams> _bloomDownsampleParams;

[[vk::binding(0,0)]]
Sampler2D _srcTexture; // rendered texture with shadows and lighting 

[[vk::binding(1,0)]]
writeonly RWTexture2D _downSampleResult; 

[[shader("compute")]]
[numthreads(8, 8, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    uint2 dstCoord = threadId.xy;
    float2 texCoord = (dstCoord + 0.5) / float2(_bloomDownsampleParams.src_dst_resolution.z, _bloomDownsampleParams.src_dst_resolution.w);

    float2 srcResolution = float2(_bloomDownsampleParams.src_dst_resolution.x, _bloomDownsampleParams.src_dst_resolution.y);
    float2 srcTexelSize = 1.0 / srcResolution;

    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    float2 srcTextureSize = 1.0 / srcResolution;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = _srcTexture.Sample( float2(texCoord.x - 2*x, texCoord.y + 2*y)).rgb;
    float3 b = _srcTexture.Sample( float2(texCoord.x,       texCoord.y + 2*y)).rgb;
    float3 c = _srcTexture.Sample( float2(texCoord.x + 2*x, texCoord.y + 2*y)).rgb;

    float3 d = _srcTexture.Sample( float2(texCoord.x - 2*x, texCoord.y)).rgb;
    float3 e = _srcTexture.Sample( float2(texCoord.x,       texCoord.y)).rgb;
    float3 f = _srcTexture.Sample( float2(texCoord.x + 2*x, texCoord.y)).rgb;

    float3 g = _srcTexture.Sample( float2(texCoord.x - 2*x, texCoord.y - 2*y)).rgb;
    float3 h = _srcTexture.Sample( float2(texCoord.x,       texCoord.y - 2*y)).rgb;
    float3 i = _srcTexture.Sample( float2(texCoord.x + 2*x, texCoord.y - 2*y)).rgb;

    float3 j = _srcTexture.Sample( float2(texCoord.x - x, texCoord.y + y)).rgb;
    float3 k = _srcTexture.Sample( float2(texCoord.x + x, texCoord.y + y)).rgb;
    float3 l = _srcTexture.Sample( float2(texCoord.x - x, texCoord.y - y)).rgb;
    float3 m = _srcTexture.Sample( float2(texCoord.x + x, texCoord.y - y)).rgb;

    float3 downSample = e* 0.125;
    downSample += (a+c+g+i)*0.03125;
    downSample += (b+d+f+h)*0.0625;
    downSample += (j+k+l+m)*0.125;

    _downSampleResult[int2(dstCoord.xy)].xyz = downSample; 

}

