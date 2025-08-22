static const uint NUM_HISTOGRMA_BINS = 256;

struct LuminanceAverageParameters{
    float pixelCount;
    float minLogLuminance;
    float logLuminanceRange;
    float timeDelta;
    float tau;
}

[[vk::push_constant]]
LuminanceAverageParameters _luminanceAverageParams;

[[vk::binding(0, 0)]]
RWStructuredBuffer<uint> _luminanceHistogram;

[[vk::binding(1, 0)]]
RWTexture2D<float> LuminanceOutput;

groupshared uint HistogramShaderd[NUM_HISTOGRMA_BINS];

[[shader("compute")]]   
[[numthreads(256, 1, 1)]]
void computeMain(uint groupIndex : SV_GroupIndex){
    uint countForThisBin = _luminanceHistogram[groupIndex];
    HistogramShaderd[groupIndex] = countForThisBin * groupIndex;

    GroupMemoryBarrierWithGroupSync();

    _luminanceHistogram[groupIndex] = 0;

    [unroll]
    for(uint cutoff = (NUM_HISTOGRMA_BINS >> 1); cutoff > 0; cutoff >>=1){
        if(groupIndex < cutoff){
            HistogramShaderd[groupIndex] += HistogramShaderd[groupIndex + cutoff];
        }

        GroupMemoryBarrierWithGroupSync();
    }

    if(groupIndex == 0){
        float weightedLogAverage = (HistogramShaderd[0] / max(_luminanceAverageParams.pixelCount - (float)countForThisBin, 1.0)) - 1.0;
        
        // transfer histogram values to actual luminance
        float weightedAverageLuminance = exp2(((weightedLogAverage / 254.0) * _luminanceAverageParams.logLuminanceRange) + _luminanceAverageParams.minLogLuminance);
        
        // retrieve luminance from the last frame 
        float luminanceLastFrame = LuminanceOutput[uint2(0, 0)];
        
        float adaptedLuminance = luminanceLastFrame + (weightedAverageLuminance - luminanceLastFrame) * (1 - exp(-_luminanceAverageParams.timeDelta * _luminanceAverageParams.tau));
        
        LuminanceOutput[uint2(0, 0)] = adaptedLuminance;

    }
}   