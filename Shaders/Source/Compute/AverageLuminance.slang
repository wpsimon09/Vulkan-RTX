static const uint NUM_HISTOGRMA_BINS = 256;

struct LuminanceAverageParameters{
    uint pixelCount;
    float minLogLuminance;
    float logLuminanceRange;
    float timeDelta;
    float tau;
}

[[vk::push_constant]]
LuminanceAverageParameters _luminanceAverageParams;

[[vk::binding(0, 0)]]
readonly ByteAddressBuffer _luminanceHistogram;

[[vk::binding(1,0)]]
writeonly RWTexture2D LuminanceOutput;

groupshared float HistogramShaderd[NUM_HISTOGRMA_BINS];

[[shader("compute")]]   
[[numthreads(16, 16, 1)]]
void computeMain(uint groupIndex : SV_GroupIndex){
    float countForThisBin = (float)_luminanceHistogram.Load(groupIndex * 4);
    HistogramShaderd[groupIndex] = countForThisBin * (float)groupIndex;

    GroupMemoryBarrierWithGroupSync();

    [unroll]
    for(uint histogramSampleIndex = (NUM_HISTOGRMA_BINS >> 1); histogramSampleIndex > 0; histogramSampleIndex >>=1){
        if(groupIndex < histogramSampleIndex){
            HistogramShaderd[groupIndex] += HistogramShaderd[groupIndex + histogramSampleIndex];
        }

        GroupMemoryBarrierWithGroupSync();
    }

    if(groupIndex == 0){
        float weightedLogAverage = (HistogramShaderd[0].x / max((float)_luminanceAverageParams.pixelCount - countForThisBin, 1.0)) - 1.0;
        float weightedAverageLuminance = exp2(((weightedLogAverage / 254.0) * _luminanceAverageParams.logLuminanceRange) + _luminanceAverageParams.minLogLuminance);
        float luminanceLastFrame = LuminanceOutput[uint2(0, 0)].x;
        float adaptedLuminance = luminanceLastFrame + (weightedAverageLuminance - luminanceLastFrame) * (1 - exp(_luminanceAverageParams.timeDelta * _luminanceAverageParams.tau));
        LuminanceOutput[uint2(0, 0)] = adaptedLuminance;
        
    }
}   