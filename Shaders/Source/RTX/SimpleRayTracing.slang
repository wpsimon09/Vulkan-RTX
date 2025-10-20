import Light;
import Utils;
import RayTracingData;
import Material;
import Surface;
import RandomValues;
import Core;
import RayTracingUtils;
import Camera;

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



struct RayPayloadShadow {
    bool inShadow;
}

[[shader("raygeneration")]]
void rayGenMain() {
    float2 dims;

    _film.GetDimensions(dims.x, dims.y);

    // this is the pixel ID
    uint2 threadIdx = DispatchRaysIndex().xy;
    uint2 launchSize = DispatchRaysDimensions().xy;

    float2 pixel = float2(DispatchRaysIndex().xy);
    float2 resolution = float2(DispatchRaysDimensions().xy);

    pixel = ((( pixel + 0.5f) / dims) * 2.f - 1.f);

    const float2 pixelCenter = float2(threadIdx.xy) + float2(0.5, 0.5);
    
    // get position on the screen inf 0, 1 interval
    const float2 inUv = pixelCenter / float2(launchSize);

    float2 d = inUv * 2.0 - 1.0;

    uint pixelIndex = threadIdx.y * launchSize.x + threadIdx.x;

    uint randomSeed = tea(pixelIndex, (uint)globalData.currentFrame);

    
    float3 totalRadiance = 0.0f;

    float3 camRight = globalData.view._m00_m10_m20;
    float3 camUp = globalData.view._m01_m11_m21;

    float aparatureSize = globalData.viewParams.x; // bigger apprature, bigger blur
    float focalLenth = globalData.viewParams.y; // position of the focuse plane

    RayPayLoad payload;

    switch((ERenderOutputRTX)globalData.renderOutputRTX){
        case ERenderOutputRTX::PathTraced:{

            //=========================
            // Full shading is enabled 
            for (int i = 0; i < globalData.raysPerPixel; i++) {
                
                 
                var camera = ProjectiveCamera(globalData.cameraPostiion.w, dims, globalData.projection, globalData.view, globalData.inverseView);
                
                var ray = camera.GenerateRayToScene(pixel, randomSeed);

                // RESET PAY LOAD FOR EACH RAY 
                payload.incomingLightColour = float3(0.0);
                payload.hasHit = true;
                payload.randomContext = randomSeed;
                payload.depth = 0;
                payload.rayColour = float3(1.0);
                payload.incomingLightColour = float3(0.0);
    
                float3 rayRadiacne = float3(0.0);
                int numSamples = 1;
                for (; payload.depth < globalData.reccursionDepth; payload.depth++) {
    
                    TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);
                    rayRadiacne += payload.incomingLightColour;
                    
                    if (!payload.hasHit){
                        break;
                    } 
                    numSamples++;
                    ray.Origin = payload.rayOrigin;
                    ray.Direction = payload.rayDirection;
                }
                totalRadiance += (1/(float)numSamples) * rayRadiacne;
            }
            
            // write to total radiance to separate buffer, in case i will need it later, most probably I wont and i am just wasting memmory here :D
            _film[int2(threadIdx.xy)] = float4(totalRadiance / globalData.raysPerPixel, 1.0);
            
            if(globalData.accumulateFrames){
                float weight = max(1.0 / (globalData.currentFrame + 1), 0);
                float3 accumulatedAverage = globalData.currentFrame > 0 ? _accumulationImage[int2(threadIdx.xy)].rgb * (1 - weight) + (totalRadiance/globalData.raysPerPixel) * weight : 0.0;
                // idk if reading is faster then calculating the total radiance, so I will just leave it like this and later check for preformance
                _accumulationImage[int2(threadIdx.xy)] = float4(globalData.currentFrame > 0 ? accumulatedAverage : totalRadiance / globalData.raysPerPixel, 1.0);
            }
            else{
                _accumulationImage[int2(threadIdx.xy)] = float4(totalRadiance / globalData.raysPerPixel, 1.0);
            }
        

            break;
        }
        case ERenderOutputRTX::Normals: {
            
            
            var camera = ProjectiveCamera(globalData.cameraPostiion.w, dims, globalData.projection, globalData.view, globalData.inverseView);
                
            var ray = camera.GenerateRayToScene(d, randomSeed);
            
            let N = TraceSingleRay(ray, _TLAS, randomSeed).normal;
            
            _accumulationImage[int2(threadIdx.xy)] = float4(N,1.0) ;
            
            break;
        }
        case ERenderOutputRTX::PDF: {
            
            var camera = ProjectiveCamera(globalData.cameraPostiion.w, resolution, globalData.projection, globalData.view, globalData.inverseView);
            var ray = camera.GenerateRayToScene(pixel, randomSeed);;
            
            let PDF = TraceSingleRay(ray, _TLAS, randomSeed).PDF;
            
            _accumulationImage[int2(threadIdx.xy)] = float4(float3(PDF), 1.0) ;
            
            break;
        }
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

    payload.normal = float3(0.0f);
    payload.incomingLightColour += (((lerp(GroundColor, skyGradient, groundToSkyT) + sun * (float)(groundToSkyT >= 1))) * payload.rayColour) * _lightInfo.dirLight.intensity.w;
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
    
    
    float3 wo = -WorldRayDirection();
    float3 V = normalize(globalData.cameraPostiion.xyz - g.shadingPosition);
    if (!shadowPayLoad.inShadow) {
        
        float3 directLightContribution = Le * m.Evaluate(wo, L, g.shadingNormal);
        
        payLoad.incomingLightColour += directLightContribution ;
    }

    //==============================
    // Path tracing 


    float3 F0 = lerp(float3(0.04), m.albedo.xyz, m.metallnes);
    float3 Ks =  F0;
    float3 Kd = (1.0 - Ks) * (1.0 - m.metallnes);

    float p_specular = max(max(Ks.x, Ks.y), Ks.z);
    bool sampleSpecular = RandomValue(payLoad.randomContext) < p_specular;

    float PDF = 1.0;
    float3 wi;
    float3 f = {};
    
    float3 emmittedLight = m.emission.xyz * m.emission.w;
    payLoad.incomingLightColour += emmittedLight * payLoad.rayColour;
    
    if( sampleSpecular){
        wi = m.SampleGGX_VND(wo, g, PDF,  payLoad.randomContext);
        f = m.EvaluateSpecular(wo, wi, g.shadingNormal, PDF);
    }else{
        wi = m.SampleLambertianDiffuse(wo, g, PDF, payLoad.randomContext);
        f = m.EvaluateDiffuse(wo, wi, g.shadingNormal, PDF); 
    }

    if(globalData.accumulateFrames){
        payLoad.rayColour *= f;
    }else{
        payLoad.rayColour *= f/PDF;
    }
    payLoad.normal = g.shadingNormal;
    payLoad.rayOrigin = g.shadingPosition;    
    payLoad.rayDirection = wi;
    payLoad.PDF = PDF;
    

    // other direction
    payLoad.hasHit = true;

}