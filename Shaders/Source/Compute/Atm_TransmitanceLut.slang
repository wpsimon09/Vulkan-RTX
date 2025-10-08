import Utils;
import Atmosphere;
import AtmosphereParameters;

static const int STEP_COUNT = 50;

[[vk::push_constant]]
AtmosphereParamsCPU atmosphereParams;

RWTexture2D<float4> transmittanceLut;

[shader("compute")]
[numthreads(16,16,1)]
void computeMain(int3 threadIdx: SV_DispatchThreadID){
   int width, height;
   transmittanceLut.GetDimensions(width, height);

   AtmosphereParamsGPU atmos = atmosphereParams.Parse();
   atmos.transmitanceTextureWidth = width;
   atmos.transmitanceTextureHeight = height;

   if(threadIdx.x >= width || threadIdx.y >= height)
      return;

   float2 uv;
   uv.x = (threadIdx.x + 0.5f) / width;
   uv.y = (threadIdx.y + 0.5f) / height; 


   float3 transmitance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(atmos, uv);

   transmittanceLut[threadIdx.xy] = float4(transmitance, 1.0);
}