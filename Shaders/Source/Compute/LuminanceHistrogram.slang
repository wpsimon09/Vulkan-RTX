static const uint NUM_HISTOGRMA_BINS = 256;


struct LuminanceHistrogramParameters{
    float width;
    float height;
    float minLogLuminance;
    float oneOverLogLuminanceRange;
};


[[vk::push_constant]]
LuminanceHistrogramParameters _luminanceHistrogramParams;

[[vk::binding(0,0)]]
Texture2D HDRTexture;

[[vk::binding(1,0)]]
RWByteAddressBuffer LuminanceHistrogram;

groupshared uint HistogramShared[NUM_HISTOGRMA_BINS];

float GetLuminance(float3 hdrColour){
    return dot(hdrColour, float3(0.2127f, 0.7152f, 0.0722f));
}

uint HDRToHistogramBin(float3 hdrColour){
    float luminance = GetLuminance(hdrColour);

    if(luminance < 0.005){
        return 0;
    }

    float logLuminance = clamp((log2(luminance) - _luminanceHistrogramParams.minLogLuminance) * _luminanceHistrogramParams.oneOverLogLuminanceRange, 0.0, 1.0);
    return (uint)(logLuminance * 254.0 + 1.0);
}

[[shader("compute")]]
[[numthreads(16, 16, 1)]]
void computeMain(uint groupIndex : SV_GroupIndex, uint3 threadId : SV_DispatchThreadID){    
    HistogramShared[groupIndex] = 0;

    GroupMemoryBarrierWithGroupSync();

    if(threadId.x < _luminanceHistrogramParams.width && threadId.y < _luminanceHistrogramParams.height){
        float3 hdrColour = HDRTexture.Load(int3(threadId.xy, 0)).rgb;
        uint binIndex = HDRToHistogramBin(hdrColour);
        InterlockedAdd(HistogramShared[binIndex], 1);
    }

    GroupMemoryBarrierWithGroupSync();

    LuminanceHistrogram.InterlockedAdd(groupIndex * 4, HistogramShared[groupIndex]);
}

