import Light;

struct GlobalData {
    column_major float4x4 projection;
    column_major float4x4 view;
    float4x4 inverseView;

    float4 lightPosition;
    float4 cameraPostiion;
    float4 viewParams; // xy- camera plane width and height,z - nerPlane, w - focal length

    float useEnvLighting;
    float padding2;
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
ConstantBuffer<Sampler2D> _film;

[[shader("raygeneration")]]
void mainRayGen() {

    // this is the pixel ID 
    uint2 threadIdx = DispatchRaysIndex().xy;
    
    if (threadIdx.x >= (int)globalData.screenSize.x) return;
    if (threadIdx.y >= (int)globalData.screenSize.y) return;

    // Calculating ray direciton
    float frameWidth = globalData.screenSize.x;


    RayDesc ray;
    ray.Origin = 
    TraceRay
}