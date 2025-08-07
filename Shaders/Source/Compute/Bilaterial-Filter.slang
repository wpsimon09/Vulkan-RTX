import Core;

[[vk::binding(0,0)]]
ConstantBuffer<GlobalData> _globalData;

[[vk::binding(1, 0)]]
Sampler2D _noisyImage;

[[vk::binding(2, 0)]]
RWTexture2D _denoisedImage;

struct FilterParameters{
   float sigma; // spatial factor 
   float BSigma; // weight factor 
   float width;
   float height;
};
static const int MSize = 9;

[[vk::push_constant]]
ConstantBuffer<FilterParameters> _filterParameters;

float normPDF(in float x, in float sigma){
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

float normpdf3(in float3 v, in float sigma)
{
	return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

float3 bilaterialFilter(uint3 threadID){
    float3 c = _noisyImage.Sample(float2(float(threadID.x + 1) / _filterParameters.width, ((float)threadID.y + 1.0) / (float)_filterParameters.height)).xyz;

    const int kSize = (MSize - 1) / 2;
    float kernel[MSize];
    float3 final_colour = float3(0.0);

    float Z = 0.0;

    // create create the ID kernel
    for(int j = 0; j<= kSize; j++){
        kernel[kSize + j] = kernel[kSize - j] = normPDF((float)j, _filterParameters.sigma);
    }

    float3 cc;
    float factor;
    float bz = 1.0 / normPDF(0.0, _filterParameters.BSigma);

    // read the image
    for(int i= -kSize; i <= kSize; ++i){
        for( int j =-kSize; j <= kSize; ++j){
            int2 samplePos = int2(threadID.xy) + int2(i, j);
            float2 uv = float2(samplePos) / float2(_filterParameters.width, _filterParameters.height);
            uv = float2(uv.x, 1.0 - uv.y);
    
            cc = _noisyImage.Sample(uv).xyz;
            factor = normpdf3(cc-c, _filterParameters.BSigma) * bz * kernel[kSize + j] * kernel[kSize + 1];
            Z+= factor;
            final_colour += factor * cc;

        }
    }

    return final_colour / Z;

}

[[shader("compute")]]
[numthreads(16, 16, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID){
    _denoisedImage[threadID.xy].xyz = bilaterialFilter(threadID).xyz;
}