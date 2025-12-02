import Core;
import RandomValues;
import Utils;

Sampler2D _normalMap;
Sampler2D _posMap;

RWTexture2D _aoMap;
RaytracingAccelerationStructure _tlas;

public struct AoOcclusionParameters{
    float radius;
    float sampleCount;
    float strenght;
    float currentFrame; 
}

[[vk::push_constant]]
public AoOcclusionParameters _aoParameters;

void compute_default_basis(const float3 normal, out float3 x, out float3 y, out float3 z)
{
  // ZAP's default coordinate system for compatibility
  z              = normal;
  const float yz = -z.y * z.z;
  y = normalize(((abs(z.z) > 0.99999f) ? float3(-z.x * z.y, 1.0f - z.y * z.y, yz) : float3(-z.x * z.z, yz, 1.0f - z.z * z.z)));

  x = cross(y, z);
}

[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID: SV_DispatchThreadID){
    uint2 dims;
    _aoMap.GetDimensions(dims.x, dims.y);
    float2 texCoords = (threadID.xy + 0.5) / dims;

    float3 N = _normalMap.Sample(texCoords).xyz;

    if(N.equals(float3(0.0))){
        _aoMap[threadID.xy].x = 1.0;
    }

    N = normalize(N);
    float3 X = _posMap.Sample(texCoords).xyz;
    float3 x, y, z;
    compute_default_basis(N, x,y,z);

    uint pixelId = threadID.x + threadID.y * dims.x;

    uint rng = tea(pixelId, (uint)_aoParameters.currentFrame);

    float aoFactor = 0.0;

    for(int i = 0; i < _aoParameters.sampleCount; i++){
        float r1 = RandomValue(rng);
        float r2 = RandomValue(rng);
        float sq = sqrt(1.0 - r2);

        // from spherical coordinate sysetm to cartesians, where (0, 0, 1) the 1 is normal vector 
        float3 aoDir = float3(
            cos(2 * PI * r1) * sq, 
            sin(2 * PI * r1) * sq, 
            sqrt(r2));
        
        // transform sampled direction back to the world space by rotating it 
        aoDir = aoDir.x * x + aoDir.y * y + aoDir.z * z;

        float tHit = _aoParameters.radius;
        bool occluded = IntersectsP(InitRay(X + normalize(N) * 0.1, aoDir, tHit),_tlas, tHit);
        
        aoFactor += (float)occluded; 
    }

    aoFactor = 1.0 - (aoFactor / _aoParameters.sampleCount);
    aoFactor = pow(clamp(aoFactor, 0, 1), _aoParameters.strenght);

    _aoMap[threadID.xy].x = aoFactor;
}