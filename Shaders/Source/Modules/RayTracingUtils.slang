module RTXUtils;
import Core;

public RayPayLoad TraceSingleRay(RayDesc ray,RaytracingAccelerationStructure _tlas, inout uint u){
    RayPayLoad payload = {};
    payload.radiance = float3(0.0);
    payload.hasHit = true;
    payload.randomContext = u;
    payload.depth = 0;
    payload.throughput = float3(1.0);

    TraceRay(_tlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}
