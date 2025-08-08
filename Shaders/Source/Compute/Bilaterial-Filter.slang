import Core;
import Utils;

[[vk::binding(0,0)]]
ConstantBuffer<GlobalData> _globalData;

[[vk::binding(1, 0)]]
Sampler2D _noisyImage;

[[vk::binding(2, 0)]]
RWTexture2D _denoisedImage;

[[vk::binding(3, 0)]]
Sampler2D _nomrmalMap;


struct FilterParameters{
   float sigma; // spatial factor 
   float BSigma; // weight factor 
   float width;
   float height;
};
static const int MSize = 15;

[[vk::push_constant]]
ConstantBuffer<FilterParameters> _filterParameters;

float normPDF(in float x, in float sigma){
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

float normpdf3(in float3 v, in float sigma)
{
	return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

float edge_stoping_weight (float3 center_normal, float3 sampled_normal, float phi_normal){
    // for depth similartiy those are the parameters: float center_depth, float sample_depth, float phi_z,
    //const float wZ =  exp(-abs(center_depth - sample_depth) / phi_z);

    float deltat = 1.0f - clamp(dot(center_normal, sampled_normal), 0.0 ,1.0);

    const float wNomral =  normPDF(deltat, phi_normal);
    return wNomral;
}



float3 bilaterialFilter(uint3 threadID){
    float2 centerUV = float2(float(threadID.x) / _filterParameters.width, ((float)threadID.y) / (float)_filterParameters.height);
    float3 c = _noisyImage.Sample(centerUV).xyz;
    float3 n_c = _nomrmalMap.Sample(centerUV).xyz;

    const int kSize = (MSize - 1) / 2; // kernel size in one dimension
    // SPACIAL KERNEL
    // - create create the  ID kernel, 
    // - it is onyl for the spacial part of the gaussian kernel since 
    // - bialteral is composed of two weights, special and range 
    // - 2 parts spacial and range 
    const float kernel[MSize] = {
        0.031225216, 0.033322271, 0.035206333, 0.036826804, 0.038138565, 0.039104044, 0.039695028, 0.039894000, 0.039695028, 0.039104044, 0.038138565, 0.036826804, 0.035206333, 0.033322271, 0.031225216    
    };
    
    float3 final_colour = float3(0.0);
    float total_weight = 0.0;

    float3 cc;
    float3 n_sample;
    float factor;

    // read the image
    for(int i= -kSize; i <= kSize; ++i){
        for( int j =-kSize; j <= kSize; ++j){
            int2 samplePos = int2(threadID.xy) + int2(i, j);
            float2 uv = float2(samplePos) / float2(_filterParameters.width, _filterParameters.height);
    
    
            // SAMPLE THE PRECOMPUTED GAUSIANG WEIGHT KERNEL
            // - it stores how far away are neighboring pixels and their weight according to the normal istribution
            // - samples closer to the center affect the final result mmore  
            float spacialWeight = kernel[kSize + j] * kernel[kSize + i];
            
            // RANGE KERNEL
            // - how similuar the neightboring pixels are 
            // - based on normla vector simiularity and depth similuarity
            n_sample = normalize(_nomrmalMap.Sample(uv).xyz);
            float rangeWeight =  edge_stoping_weight(n_c, n_sample, _filterParameters.BSigma);

            //rangeWeight * 
            factor = spacialWeight * rangeWeight;
            
            cc = _noisyImage.Sample(uv).xyz;

            final_colour += factor * cc;
            total_weight += factor;

        }
    }

    return final_colour /  max(total_weight, 0.00001);

}



[[shader("compute")]]
[numthreads(16, 16, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID){
    _denoisedImage[threadID.xy].xyz = bilaterialFilter(threadID).xyz;
}