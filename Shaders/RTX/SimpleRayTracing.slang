import Light;
import Utils;

struct GlobalData {
    column_major float4x4 projection;
    column_major float4x4 view;
    float4x4 inverseView;
    

    float4 lightPosition;
    float4 cameraPostiion;
    float4 viewParams; // xy- camera plane width and height,z - nerPlane, w - focal length

    float useEnvLighting;
    float reccursionDepth;;
    float2 screenSize;
};

// global data for the ray tracing 
[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> globalData;

struct LightInfo {
    DirectionalLight dirLight;
    // PointLight[10] pointLigths;
    LightArray<PointLight, 20> pointLights;
    LightArray<AreaLight, 4> areaLights;
    float4 info;
}

// light light in scene information
[[vk::binding(1, 0)]]
ConstantBuffer<LightInfo> _lightInfo;

/**
    TODO: once i have all data in single array this descriptor set will contain everything, textures, materials and per object data, objects
    cann access this information via custom index specified in the acceleration strucutre construction
*/

// top level acceleration structure
[[vk::binding(3, 0)]]
RaytracingAccelerationStructure _TLAS;

// output of the ray tracing
[[vk::binding(4, 0)]]
ConstantBuffer<RWTexture2D> _film;

struct RayPayLoad {
    float3 color;
    float3 distance;
    float3 normal;
    float3 reflector;
}

[[shader("raygeneration")]]
void mainRayGen() {

    
    // this is the pixel ID
    uint2 threadIdx = DispatchRaysIndex().xy;
    uint2 launchSize = DispatchRaysDimensions().xy;
    
    if (threadIdx.x >= (int)globalData.screenSize.x) return;
    if (threadIdx.y >= (int)globalData.screenSize.y) return;

    const float2 pixelCenter = float2(threadIdx.xy) + float2(0.5, 0.5);
    const float2 inUv = pixelCenter / float2(launchSize);


    float2 d = inUv * 2.0 - 1.0;
    // pixel position on view plane 
    float4 target = mul(inverse(globalData.projection), float4(d.x, d.y, 1.0, 1.0));

    RayDesc ray;
    ray.Origin = mul(globalData.inverseView, float4(0.0, 0.0, 0.0, 1.0)).xyz;
    ray.Direction = mul(globalData.inverseView, float4(normalize(target.xyz), 0)).xyz;
    ray.TMin = 0.0f;
    ray.TMax = 10000.0f;

    float3 totalRadiance = 0.0f;

    for (int i = 0; i < globalData.reccursionDepth; i++) {
        RayPayLoad payload;
        TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);
        float3 hitColor = payload.color;
        totalRadiance += hitColor;
     }

     _film[int2(threadIdx.xy)] = float4(totalRadiance, 0.0);
}

[[shader("miss")]]
void mainMiss(inout RayPayLoad payload) {
    payload.color = float3(0.0, 0.0, 0.0);

}

[[shader("closesthit")]]
void mainClosestHit(inout RayPayLoad payLoad, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 hitLocation = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    payLoad.color = hitLocation;
}