import Core;
import Utils;

ConstantBuffer<Texture2D> _inputImage;
ConstantBuffer<RWTexture2D> _outputImage;



float conv(in float kernel[9], in float data[9], in float denom, in float offset)
{
   float res = 0.0;
   for (int i=0; i<9; ++i)
   {
      res += kernel[i] * data[i];
   }
   return saturate(res/denom + offset);
}

[[shader("compute")]]
[numthreads(16, 16, 1)] // number of thread in the work group 
void computeMain(uint3 threadID : SV_DispatchThreadID){
    float imageData[9];
	// Fetch neighbouring texels
	int n = -1;
	for (int i=-1; i<2; ++i)
	{
		for(int j=-1; j<2; ++j)
		{
			n++;
			float3 rgb = _inputImage[uint2(threadID.x + i, threadID.y + j)].rgb;
			imageData[n] = (rgb.r + rgb.g + rgb.b) / 3.0;
		}
	}

	float kernel[9];
	kernel[0] = -1.0/8.0; kernel[1] = -1.0/8.0; kernel[2] = -1.0/8.0;
	kernel[3] = -1.0/8.0; kernel[4] =  1.0;     kernel[5] = -1.0/8.0;
	kernel[6] = -1.0/8.0; kernel[7] = -1.0/8.0; kernel[8] = -1.0/8.0;

	float4 res = float4(conv(kernel, imageData, 0.1, 0.0).xxx, 1.0);

	_outputImage[int2(threadID.xy)] = res;
}