struct BloomSettings
{
    float4 src_dst_dims;
    float bloomStrength;
    bool useDirtTexture;
    float padding, padding2;
}

[[vk::push_constant]]
BloomSettings bloomSettings;

Sampler2D _bloomUpSampleResult_A;

Sampler2D hdrTexture;

writeonly RWTexture2D _bloomFinalResult;

//Sampler2D _dirtTexture;

[[shader("compute")]]
[numthreads(16, 16, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    int2 dstWriteCoords = threadId.xy;

    float2 srcDim = bloomSettings.src_dst_dims.xy;
    float2 dstDim = bloomSettings.src_dst_dims.zw;

    float2 texCooreds = (dstWriteCoords + 0.5) / dstDim; // convert to normlaised 0,1 space 
 
    float4 HDRColour = hdrTexture.Sample(texCooreds);
    float4 BloomResult = _bloomUpSampleResult_A.Sample(texCooreds);

    float4 result = lerp(HDRColour, BloomResult, bloomSettings.bloomStrength);

    _bloomFinalResult[dstWriteCoords] = result;
}