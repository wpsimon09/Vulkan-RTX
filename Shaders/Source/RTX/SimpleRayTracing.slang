import Light;
import Utils;
import RayTracingData;
import Material;
import Surface;
import RandomValues;
import Core;

// global data for the ray tracing 
[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> globalData;

// light light in scene information
[[vk::binding(1, 0)]]
ConstantBuffer<LightInfo> _lightInfo;

// top level acceleration structure
[[vk::binding(3, 0)]]
RaytracingAccelerationStructure _TLAS;

// output of the ray tracing
[[vk::binding(4, 0)]]
ConstantBuffer<RWTexture2D> _film;

[[vk::binding(6, 0)]]
ConstantBuffer<RWTexture2D> _accumulationImage;

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

    uint pixelIndex = threadIdx.y * launchSize.x + threadIdx.x;

    uint randomSeed = tea(pixelIndex, (uint)globalData.currentFrame);

    // pixel position on view plane 
    float4 target = mul(globalData.invProjection, float4(d.x, d.y, 1.0, 1.0));
    
    float3 totalRadiance = 0.0f;

    RayPayLoad payload;
    payload.incomingLightColour = float3(0.0);
    payload.hasHit = true;
    payload.randomContext = randomSeed;
    payload.depth = 0;
    payload.rayColour = float3(1.0);

    float3 camRight = globalData.view._m00_m10_m20;
    float3 camUp = globalData.view._m01_m11_m21;

    float aparatureSize = globalData.viewParams.x; // bigger apprature, bigger blur
    float focalLenth = globalData.viewParams.y; // position of the focuse plane

    if((ERenderOutput)globalData.renderOutput != ERenderOutput::NormalVectors){
        
        //=========================
        // Full shading is enabled 
        for (int i = 0; i < globalData.raysPerPixel; i++) {

            
            //========================================================================================c
            // get initial jitter samples 
            float2 jitter = RandomPointInCircle(randomSeed) * 0.2 / launchSize.x;
            float2 deFocusJitter = RandomPointInCircle(randomSeed) * aparatureSize;
            float3 jitteredTarget = target.xyz + camRight * jitter.x + camUp * jitter.y;
            

            //========================================================================================c
            // define ray origin in the camera space and translate it to the worlds space 
            float3 rayOriginWS = mul(globalData.inverseView, float4(0.0, 0.0, 0.0, 1.0)).xyz;

            //===========================================================================
            // jitter the ray origin 
            float3 jitteredRayOriginWS =rayOriginWS + camRight * deFocusJitter.x +  camUp * deFocusJitter.y; // offset the ray origin
            
            //======================================================
            // calulate where the ray will shoot in the world space 
            float3 rayDirecionWS = normalize(mul(globalData.inverseView, float4(target.xyz, 0.0))).xyz;
            
            //======================================================
            // find the focal point, the point where objects are in focuse 
            float3 focalPoint = rayOriginWS + rayDirecionWS * focalLenth;
            
            //======================================================
            // descibbe ray
            RayDesc ray;
            ray.Origin = jitteredRayOriginWS;
            ray.Direction = normalize(focalPoint - ray.Origin); // direction from jittered origin to the focal point 
            ray.TMin = 0.001f;
            ray.TMax = 10000.0f;


            payload.incomingLightColour = float3(0.0);
            float3 rayRadiacne = float3(0.0);
            for (; payload.depth < globalData.reccursionDepth; payload.depth++) {

                TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);
                rayRadiacne += payload.incomingLightColour;
                
                if (!payload.hasHit)
                    break;
                
                ray.Origin = payload.rayOrigin;
                ray.Direction = payload.rayDirection;
            }
            totalRadiance += rayRadiacne;
        }
            float weight = max(1.0 / (globalData.currentFrame + 1), 0);
    
        // write to total radiance to separate buffer, in case i will need it later, most probably I wont and i am just wasting memmory here :D
        _film[int2(threadIdx.xy)] = float4(totalRadiance / globalData.raysPerPixel, 1.0);
    
        // idk if reading is faster then calculating the total radiance, so I will just leave it like this and later check for preformance
        float3 accumulatedAverage = globalData.currentFrame > 0 ? _accumulationImage[int2(threadIdx.xy)].rgb * (1 - weight) + (totalRadiance/globalData.raysPerPixel) * weight : 0.0;
    
        _accumulationImage[int2(threadIdx.xy)] = float4(globalData.currentFrame > 0 ? accumulatedAverage : totalRadiance / globalData.raysPerPixel, 1.0);
    }
    else{

        RayDesc ray;
        ray.Origin = mul(globalData.inverseView, float4(0.0, 0.0, 0.0, 1.0)).xyz;
        ray.Direction = mul(globalData.inverseView, float4(normalize(target.xyz), 0.0)).xyz;
        //ray.Direction = normalize(worldSpaceDirection).xyz;
        ray.TMin = 0.001f;
        ray.TMax = 10000.0f;

        TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);
        _accumulationImage[int2(threadIdx.xy)] = float4(payload.normal,1.0);

    }
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
    float sun = pow(max(0, dot(ray_d, normalize(_lightInfo.dirLight.direction.xyz))), 64.0) * _lightInfo.dirLight.intensity.w;

    //payload.incomingLightColour = float3(0.f);
    payload.normal = float3(0.0f);
    payload.incomingLightColour += (lerp(GroundColor,   skyGradient, groundToSkyT) + sun * (float)(groundToSkyT >= 1)) * payload.rayColour ;
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
    CookTorrence_GGX m = GetMaterial(InstanceID(), g);


    //========================
    // direct light
    float3 L = _lightInfo.dirLight.direction.xyz;
    float3 Le = (_lightInfo.dirLight.intensity.xyz * _lightInfo.dirLight.intensity.w).xyz;
    
    float3 albedo = m.albedo.xyz; // float3(0.2, 0.2, 1.0);
    
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
    
    
    float3 V = normalize(globalData.cameraPostiion.xyz - g.shadingPosition);
    if (!shadowPayLoad.inShadow) {
        
        float3 directLightContribution = Le * m.Evaluate(V, L, g.shadingNormal);
        
        payLoad.incomingLightColour += payLoad.rayColour * directLightContribution;
    }

    //==============================
    // Path tracing 

    float3 wo = -WorldRayDirection();


    float3 F0 = lerp(float3(0.04), m.albedo.xyz, m.metallnes);
    float3 Ks =  F0;
    float3 Kd = (1.0 - Ks) * (1.0 - m.metallnes);

    float p_specular = max(max(Ks.x, Ks.y), Ks.z);
    bool sampleSpecular = RandomValue(payLoad.randomContext) < p_specular;

    float PDF;
    float3 wi;

    if( sampleSpecular){
        wi = m.SampleGGX_VND(wo, g, PDF,  payLoad.randomContext);
        float3 f = m.EvaluateSpecular(wo, wi, g.shadingNormal);
        payLoad.rayColour *= f / max(PDF, 1e-5);
    }else{
        wi = m.SampleLambertianDiffuse(wo, g, PDF,  payLoad.randomContext);
        float3 f = m.EvaluateDiffuse(wo, wi, g.shadingNormal);
        payLoad.rayColour *= f / max(PDF, 1e-5); 
    }

    // calculate next bounce direction of the ray 
    payLoad.rayOrigin = g.shadingPosition;    
    payLoad.rayDirection = wi;


    // other direction
    float3 emmittedLight = m.emission.xyz * m.emission.w;
    payLoad.normal = g.shadingNormal;
    payLoad.incomingLightColour += emmittedLight * payLoad.rayColour;
    payLoad.rayColour *= m.Evaluate(wi, wo, g.shadingNormal);
    payLoad.hasHit = true;

}