import Utils;
import Atmosphere;

static const int STEP_COUNT = 50;


RWTexture2D<float4> transmittanceLut;

[shader("compute")]
[numthreads(16,16,1)]
void computeMain(int3 threadIdx: SV_DispatchThreadID){
    int width, height;
    transmittanceLut.GetDimensions(width, height);

     if(threadIdx.x >= width || threadIdx.y >= height)
        return;

    
}
