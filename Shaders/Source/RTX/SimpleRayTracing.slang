import Light;
import Utils;
import RayTracingData;
import Material;
import Surface;
import RandomValues;
import Core;
import RayTracingUtils;
import Camera;
import ColourSpaceConversions;
import BxDF;

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

[[vk::binding(8,0)]]
SamplerCube envMap;

struct RayPayloadShadow {
    bool inShadow;
}

static const int MIN_BOUNCES_TO_TERMINATE = 2;

[[shader("raygeneration")]]
void rayGenMain() {
    float2 dims;

    _film.GetDimensions(dims.x, dims.y);

    // this is the pixel ID
    uint2 threadIdx = DispatchRaysIndex().xy;
    uint2 launchSize = DispatchRaysDimensions().xy;
    uint pixelIndex = threadIdx.y * launchSize.x + threadIdx.x;
    uint randomSeed = tea(pixelIndex, (uint)globalData.currentFrame);


    float2 pixelCenter = float2(threadIdx.xy) + float2(0.5, 0.5);
    float2 offset = float2(RandomValue(randomSeed), RandomValue(randomSeed));
    
    // get position on the screen inf 0, 1 interval
    float2 jitteredCenter = float2(threadIdx.xy) + offset;
    float2 inUv = jitteredCenter / float2(launchSize);
    float2 d = inUv * 2.0 - 1.0;

    ThinCameraParameters cameraParameters;
    cameraParameters.aparatureSize = globalData.viewParams2.z;
    cameraParameters.imagePlaneDistance  = globalData.viewParams.x;
    cameraParameters.focalLength = globalData.viewParams.y;
    
    float tanFov2 = globalData.viewParams2.x;
    float aspect = globalData.viewParams2.y;

    ECameraType cameraType = (ECameraType)globalData.viewParams2.w;

    switch((ERenderOutputRTX)globalData.renderOutputRTX){
        case ERenderOutputRTX::PathTraced:{

            var camera = ProjectiveCamera(tanFov2, aspect, globalData.view, globalData.inverseView);
            float3 acumulatedRadiance = {0.0};
            
            //=========================
            // Full shading is enabled 
            for (int i = 0; i < globalData.raysPerPixel; i++) {
                float2 offset = float2(RandomValue(randomSeed), RandomValue(randomSeed));
                float2 jitteredCenter = float2(threadIdx.xy) + offset;
                float2 sampleUv = jitteredCenter / float2(launchSize);
                float2 sampleD = sampleUv * 2.0 - 1.0;
                
                var ray = camera.CameraToWorld(camera.GenerateRayToScene(sampleD, randomSeed));
                
                RayPayLoad payload = {};
                payload.randomContext = randomSeed;
                
                int numSamples = 1;
                for (; payload.depth < globalData.reccursionDepth; payload.depth++) {
    
                    if(globalData.useSER){
                        HitObject hitObj = HitObject::TraceRay(_TLAS,RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);
                        ReorderThread(hitObj);
                        if(hitObj.IsHit()){
                            payload.hitObject = hitObj;
                        }
                        HitObject::Invoke(_TLAS, hitObj, payload);
                    }else{
                        TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);
                    }

                    if (!payload.hitSurface.hasValue){
                        break;
                    } 
                    
                    // russian rullete
                    if(payload.depth > MIN_BOUNCES_TO_TERMINATE){
                        float rr_p = min(0.95f, RGBToLuminance(payload.throughput));
                        if(rr_p < RandomValue(payload.randomContext)){
                            break;
                        }else{
                            payload.throughput /= rr_p;
                        }
                    }
                
                    numSamples++;
                    ray.Origin = payload.rayOrigin;
                    ray.Direction = payload.rayDirection;
                }
    
                if(IsNan(payload.radiance)){
                    acumulatedRadiance += float3(1.0, 0.0, 0.0) ;                    
                }else{  
                    acumulatedRadiance += payload.radiance ;
                }

            }
            
            camera.film = float4(acumulatedRadiance, 1.0);

            float4 pixelColour = camera.film / globalData.raysPerPixel;
        
            _film[int2(threadIdx.xy)] = pixelColour;
    

            if(globalData.accumulateFrames){
                float4 previousColour =  _accumulationImage[int2(threadIdx.xy)];
    
                float4 accumulatedColour = previousColour + pixelColour;
             
                _accumulationImage[int2(threadIdx.xy)] = globalData.currentFrame > 0 ? accumulatedColour : pixelColour, 1.0;  
            }
            else{                
                _accumulationImage[int2(threadIdx.xy)] = pixelColour;
            }
            

            break;
        }
        case ERenderOutputRTX::Normals: {    
            var camera = ProjectiveCamera(globalData.viewParams2.x, globalData.viewParams2.y, globalData.view, globalData.inverseView);                
            var ray = camera.CameraToWorld(camera.GenerateRayToScene(d, randomSeed));
            
            float3 worldNormal = {};
            if(let s = TraceSingleRay(ray, _TLAS, randomSeed).hitSurface){
                worldNormal = s.shadingNormal;
            }
            
            _accumulationImage[int2(threadIdx.xy)] = float4(worldNormal, 1.0) ;
      
            break;
        }
        case ERenderOutputRTX::PDF: {
            var camera = ProjectiveCamera(globalData.viewParams2.x, globalData.viewParams2.y, globalData.view, globalData.inverseView);
            var ray = camera.CameraToWorld(camera.GenerateRayToScene(d, randomSeed));
            
            float3 albedo = float3(0.0f);
            
            if(let s = TraceSingleRay(ray, _TLAS, randomSeed).brdfContext){
                albedo = float3(s.alpha);
            }
            
            _accumulationImage[int2(threadIdx.xy)] = float4(albedo, 1.0) ;
            
            break;
        }
    }
}

[[shader("miss")]]
void missMain(inout RayPayLoad payload) {
    float3 skyColour =  float3(0.2);//envMap.Sample(normalize(payload.rayDirection.xyz)).xyz;
    payload.radiance += payload.throughput * skyColour;
    payload.randomContext = payload.randomContext;
    payload.hitSurface = {};
}

[[shader("miss")]]
void missMain2(inout RayPayloadShadow payload) {
    payload.inShadow = false;
}

[[shader("closesthit")]]
void closestHitMain(inout RayPayLoad payLoad, in BuiltInTriangleIntersectionAttributes attr)
{
    SurfaceGeometry g;
    float3 wo = float3(0.0);
    PBRMaterial m;
    if(globalData.useSER){
        g = GetSurfaceInfoFromHitObject(payLoad.hitObject);
        wo = -payLoad.hitObject.GetObjectRayDirection();
        m = GetMaterial(payLoad.hitObject.GetInstanceID(), g);
    }else{
        g = GetSurfaceInfo(PrimitiveIndex(), InstanceIndex(), attr.barycentrics, ObjectToWorld());
        wo  = -WorldRayDirection();
        m = GetMaterial(InstanceID(), g);
    }
    var bxdfContext = m.Prepare(g);

    //========================
    // direct light

    float3 L = _lightInfo.dirLight.direction.xyz;
    float3 Li = (_lightInfo.dirLight.intensity.xyz * _lightInfo.dirLight.intensity.w).xyz;
    
    RayPayloadShadow shadowPayLoad;
    shadowPayLoad.inShadow = true;

    if(dot(g.shadingNormal, wo) <0.0f){
        g.shadingNormal = -g.shadingNormal;
    }
    if(dot(g.normal, wo) < 0.0f){
        g.normal = -g.normal;
    }   
    
    if (dot(g.shadingNormal, L) > 0.0) {
        // trace the shadow rays
        RayDesc shadowRay;
        shadowRay.Direction = L;
        shadowRay.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
        shadowRay.TMax = 10000;
        shadowRay.TMin = 0.01;
        
        //TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 0, 1, shadowRay, shadowPayLoad);
    }
    if (!shadowPayLoad.inShadow && (bool)_lightInfo.dirLight.parameters.z) {
        
        var brdf = DeterministicBRDF(bxdfContext);

        float cosTheta = max(dot(g.shadingNormal, L),0.0);
        float3 directLightContribution = Li * (brdf.Evaluate(wo, L, g) * cosTheta) ;
        
        //payLoad.radiance += payLoad.throughput * directLightContribution ;
    }

    //==============================
    // Path tracing
    
    //============================
    // Account for emmision
    float3 Le = bxdfContext.emmision * bxdfContext.emissionStrength;
    payLoad.radiance += Le * payLoad.throughput;
    
    //==========================================
    // Evaluate surface scattering 

    // which brdf to use 
    var selectedBrdf = m.Sample(bxdfContext, payLoad.randomContext, g,payLoad.randomContext);
    
    let brdfSample = selectedBrdf.brdf.Sample(wo, g, payLoad.randomContext);

    if(brdfSample.hasValue){
        float3 Lo;
        float cosTheta = max(dot(g.shadingNormal, normalize(brdfSample.value.wi)),0.0);
 
        Lo = (selectedBrdf.brdf.Evaluate(wo, brdfSample.value.wi, g) * cosTheta) / brdfSample.value.PDF;

        payLoad.throughput /= selectedBrdf.p;
        payLoad.throughput *= Lo;
        payLoad.rayDirection = brdfSample.value.wi;
        
        payLoad.hitSurface = g;
        payLoad.rayOrigin = g.shadingPosition + g.shadingNormal * 0.001;    
        payLoad.depth++;
    }
    else{
        payLoad.hitSurface = {};
    }


}