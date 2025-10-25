import Light;
import Utils;
import RayTracingData;
import Material;
import Surface;
import RandomValues;
import Core;
import RayTracingUtils;
import Camera;
import ColourSpaceConversions;;

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

    switch((ERenderOutputRTX)globalData.renderOutputRTX){
        case ERenderOutputRTX::PathTraced:{

            //=========================
            // Full shading is enabled 
            for (int i = 0; i < globalData.raysPerPixel; i++) {
                
                 
                var camera = ProjectiveCamera(globalData.viewParams2.x, globalData.viewParams2.y, globalData.view, globalData.inverseView);
                
                var ray = camera.GenerateRayToScene(d, randomSeed);
                
                RayPayLoad payload = {};
                payload.randomContext = randomSeed;
                
                int numSamples = 1;
                for (; payload.depth < globalData.reccursionDepth; payload.depth++) {
    
                    
                    TraceRay(_TLAS, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);                    
                    if (!payload.hasHit){
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

                totalRadiance += (1/(float)numSamples) * payload.radiance;
            }
            
            // write to total radiance to separate buffer, in case i will need it later, most probably I wont and i am just wasting memmory here :D
            _film[int2(threadIdx.xy)] = float4(totalRadiance / globalData.raysPerPixel, 1.0);
            

            float3 pixelColour = totalRadiance/globalData.raysPerPixel;

            if(globalData.accumulateFrames){
                float weight = max(1.0 / (globalData.currentFrame + 1), 0);
                float3 accumulatedAverage = globalData.currentFrame > 0 ? _accumulationImage[int2(threadIdx.xy)].rgb * (1 - weight) + (pixelColour) * weight : 0.0;
                // idk if reading is faster then calculating the total radiance, so I will just leave it like this and later check for preformance
                _accumulationImage[int2(threadIdx.xy)] = float4(globalData.currentFrame > 0 ? accumulatedAverage : totalRadiance / globalData.raysPerPixel, 1.0);
            }
            else{
                _accumulationImage[int2(threadIdx.xy)] = float4(totalRadiance / globalData.raysPerPixel, 1.0);
            }
        

            break;
        }
        case ERenderOutputRTX::Normals: {
            
            
            var camera = ProjectiveCamera(globalData.viewParams2.x, globalData.viewParams2.y, globalData.view, globalData.inverseView);                
            var ray = camera.GenerateRayToScene(d, randomSeed);
            
            float3 worldNormal = {};
            if(let s = TraceSingleRay(ray, _TLAS, randomSeed).hitSurface){
                worldNormal = s.shadingNormal;
            }
            
            _accumulationImage[int2(threadIdx.xy)] = float4(worldNormal, 1.0) ;
      
            break;
        }
        case ERenderOutputRTX::PDF: {
            
            var camera = ProjectiveCamera(globalData.viewParams2.x, globalData.viewParams2.y, globalData.view, globalData.inverseView);
            var ray = camera.GenerateRayToScene(d, randomSeed);;
            
            float3 geometryNormal = {};
            if(let s = TraceSingleRay(ray, _TLAS, randomSeed).hitSurface){
                geometryNormal = s.normal;
            }
            
            _accumulationImage[int2(threadIdx.xy)] = float4(geometryNormal, 1.0) ;
            
            break;
        }
    }
}

[[shader("miss")]]
void missMain(inout RayPayLoad payload) {
    float3 skyColour =  float3(1.0);//envMap.Sample(normalize(payload.rayDirection.xyz)).xyz;
    payload.radiance += payload.throughput * skyColour;
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
    /*
    float3 L = _lightInfo.dirLight.direction.xyz;
    float3 Li = (_lightInfo.dirLight.intensity.xyz * _lightInfo.dirLight.intensity.w).xyz;
    
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
    if (!shadowPayLoad.inShadow && (bool)_lightInfo.dirLight.parameters.z) {
        
        float3 directLightContribution = Li * m.Evaluate(wo, L, g.shadingNormal);
        
        //payLoad.radiance += directLightContribution ;
    }
    */
    
    
    float3 wo = -WorldRayDirection();

    //==============================
    // Path tracing 
    if (dot(g.shadingNormal, wo) <= 0.0f)
        return;

    float3 F0 = lerp(float3(0.04), m.albedo.xyz, m.metallnes);
    float3 Ks =  F0;
    float3 Kd = (1.0 - Ks) * (1.0 - m.metallnes);

    float p_specular = max(max(Ks.x, Ks.y), Ks.z);
    bool sampleSpecular = false;//RandomValue(payLoad.randomContext) < p_specular;

    float PDF = 1.0;
    float3 wi;
    float3 f = {};
    
    float3 Le = m.emission.xyz * m.emission.w;
    
    if( sampleSpecular){
        wi = m.SampleGGX_VND(wo, g, PDF,  payLoad.randomContext);
        f = m.EvaluateSpecular(wo, wi, g.shadingNormal) ;
        PDF = 1;
    }else{
        wi = m.SampleLambertianDiffuse(wo, g, PDF, payLoad.randomContext);
        f = m.EvaluateDiffuse(wo, wi, g.shadingNormal); 
    }

    if(globalData.accumulateFrames){
        payLoad.throughput *= f/ PDF;
    }else{
        payLoad.throughput *= f / PDF;
    }
    payLoad.radiance += Le * payLoad.throughput;

    payLoad.hitSurface = g;
    payLoad.rayOrigin = g.shadingPosition + g.shadingNormal * 0.001;    
    payLoad.rayDirection = wi;

    // other direction
    payLoad.hasHit = true;

}