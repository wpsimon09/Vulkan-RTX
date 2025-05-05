import Light;
import Utils;
import RayTracingData;
import Material;
import Surface;
import RandomValues;

struct GlobalData {
    column_major float4x4 projection;
    column_major float4x4 view;
    float4x4 inverseView;
    float4x4 invProjection;

    float4 lightPosition;
    float4 cameraPostiion;
    float4 viewParams; // xy- camera plane width and height,z - nerPlane, w - focal length

    float raysPerPixel;
    float reccursionDepth;
    float2 screenSize;
    float currentFrame;
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
    float3 distance;
    float3 normal;
    float3 reflector;

    float3 incomingLightColour;
    float3 rayColour;

    float3 rayDirection;
    float3 rayOrigin;

    uint depth;
    uint randomContext;

    bool hasHit;
}

struct RayPayloadShadow {
    bool inShadow;
}

[[shader("raygeneration")]]
void rayGenMain() {

    
    // this is the pixel ID
    uint2 threadIdx = DispatchRaysIndex().xy;
    uint2 launchSize = DispatchRaysDimensions().xy;

    const float2 pixelCenter = float2(threadIdx.xy) + float2(0.5, 0.5);
    // get position on the screen inf 0, 1 interval
    const float2 inUv = pixelCenter / float2(launchSize);

    float2 d = inUv * 2.0 - 1.0;

    float2 pixelCoord = inUv * globalData.screenSize;
    uint pixelIndex = threadIdx.y * launchSize.x + threadIdx.x;

    pixelIndex += (uint)globalData.currentFrame * 719393;
    // pixel position on view plane 
    float4 target = mul(globalData.invProjection, float4(d.x, d.y, 1.0, 1.0));

    RayDesc ray;
    ray.Origin = mul(globalData.inverseView, float4(0.0, 0.0, 0.0, 1.0)).xyz;
    ray.Direction = mul(globalData.inverseView, float4(normalize(target.xyz), 0.0)).xyz;
    //ray.Direction = normalize(worldSpaceDirection).xyz;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0f;

    float3 totalRadiance = 0.0f;

    RayPayLoad payload;
    payload.incomingLightColour = float3(0.0);
    payload.rayColour = float3(1.0);
    payload.hasHit = true;
    payload.randomContext = pixelIndex;
    payload.depth = 0;

    for (int i = 0; i < globalData.raysPerPixel; i++) {
        float3 rayRadiacne = float3(0.0);
        for (; payload.depth < globalData.reccursionDepth && payload.hasHit; payload.depth++) {

            TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);
            rayRadiacne += payload.incomingLightColour;
            
            ray.Origin = payload.rayOrigin;
            ray.Direction = payload.rayDirection;
        }
        totalRadiance += rayRadiacne;
    }
 
    _film[int2(threadIdx.xy)] = float4(totalRadiance/globalData.raysPerPixel, 1.0);
}

[[shader("miss")]]
void missMain(inout RayPayLoad payload) {
    float3 SkyColorHorizon = float3(1.0, 1.0, 1.0);
    float3 SkyColorZenith = float3(0.7, 0.7, 0.9);
    float3 GroundColor = float3(0.4, 0.4, 0.4);

    float3 ray_d = WorldRayDirection().xyz;

    float skyGradientT = pow(smoothstep(0, 0.4, ray_d.y), 0.35);
    float groundToSkyT = smoothstep(-0.01, 0, ray_d.y);

    float3 skyGradient = lerp(SkyColorHorizon, SkyColorZenith, skyGradientT);
    float sun = pow(max(0, dot(ray_d, normalize(_lightInfo.dirLight.direction.xyz))), 200.0) * _lightInfo.dirLight.intensity.w;

    //payload.incomingLightColour = float3(0.f);
    payload.incomingLightColour += lerp(GroundColor, skyGradient, groundToSkyT) + sun * (float)(groundToSkyT >= 1) ;
    payload.randomContext = payload.randomContext;
    payload.hasHit = false;
}

[[shader("miss")]]
void missMain2(inout RayPayloadShadow payload) {
    payload.inShadow = false;
}

[[shader("closesthit")]]
void closestHitMain(inout RayPayLoad payLoad, in BuiltInTriangleIntersectionAttributes attr)
{

    SurfaceGeometry g = GetSurfaceInfo(PrimitiveIndex(), InstanceID(), attr.barycentrics);
    Material m = GetMaterial(InstanceID());

    //========================
    // direct light
    float3 L = _lightInfo.dirLight.direction.xyz;
    float3 Le = (_lightInfo.dirLight.intensity.xyz * _lightInfo.dirLight.intensity.w).xyz;

    float3 albedo = m.Values.diffuse.xyz; // float3(0.2, 0.2, 1.0);

    float3 ambient = albedo * _lightInfo.info.y; // y is the ambient strengt

    RayPayloadShadow shadowPayLoad;
    shadowPayLoad.inShadow = true;

    if (dot(g.shadingNormal, L) > 0.0) {
        // trace the shadow rays
        RayDesc shadowRay;
        shadowRay.Direction = L;
        shadowRay.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
        shadowRay.TMax = 10000;
        shadowRay.TMin = 0.01;

        TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 0, 1, shadowRay, shadowPayLoad);
    }

    if (!shadowPayLoad.inShadow) {
        float diff = max(dot(g.shadingNormal, L), 0.0);

        payLoad.incomingLightColour += (diff * Le * albedo) + ambient;
    }

    //==============================
    // Path tracing 

    // calculate next bounce direction of the ray 
    float3 diffuseDir = normalize(g.shadingNormal + RandomHemisphereDirection(g.shadingNormal, payLoad.randomContext));
    payLoad.rayDirection = diffuseDir;
    payLoad.rayOrigin = g.shadingPosition;
    
    // other direction
    float3 emmittedLight = m.Values.emissive_strength.xyz * m.Values.emissive_strength.w;

    payLoad.rayColour *= m.Values.diffuse.xyz;
    payLoad.incomingLightColour += emmittedLight * payLoad.rayColour;
    payLoad.hasHit = true;

    

}