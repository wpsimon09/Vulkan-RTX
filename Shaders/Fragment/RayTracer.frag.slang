#define PI 3.14159265359

Sphere[] BuildScene() {

    // Material definitions
    RayTracingMaterial redMaterial;
    redMaterial.color = float3(0.9, 0.0, 0.0);
    
    RayTracingMaterial greenMaterial;
    greenMaterial.color = float3(0.0, 0.9, 0.0);
    greenMaterial.smoothenss = 1.0;

    RayTracingMaterial blueMaterial;
    blueMaterial.color = float3(0.0, 0.0, 0.9);

    RayTracingMaterial yellowMaterial;
    yellowMaterial.color = float3(1.0, 1.0, 0.0);

    RayTracingMaterial grayMaterial;
    grayMaterial.color = float3(0.5, 0.5, 0.5);

    RayTracingMaterial purpleMaterial;
    purpleMaterial.color = float3(0.6, 0.0, 0.6);

    RayTracingMaterial orangeMaterial;
    orangeMaterial.color = float3(1.0, 0.5, 0.0);

    RayTracingMaterial sunMaterial;
    sunMaterial.color = float3(0.0); // Bright yellow
    sunMaterial.emissiveColour = float3(1.0, 1.0, 1.0);
    sunMaterial.emissionStrength = 20;

    RayTracingMaterial lightMaterial;
    lightMaterial.color = float3(1.0, 1.0, 1.0); // White light

    RayTracingMaterial floorMaterial;
    floorMaterial.color = float3(0.8); // Dark gray for the floor
    //floorMaterial.smoothenss = 0.9;

    // Sphere definitions
    Sphere floorSphere;
    floorSphere.position = float3(0.0, -50.0, 0.0); // Below the scene
    floorSphere.radius = 50.0;                      // Huge sphere for the floor
    floorSphere.mat = floorMaterial;

    Sphere sunSphere;
    sunSphere.position = float3(0.0, 100.0, -280.0); // High above the scene
    sunSphere.radius = 80.0;                        // Large sphere for the sun
    sunSphere.mat = sunMaterial;

    Sphere sphere1;
    sphere1.position = float3(15.0, -5.0, -25.0);
    sphere1.radius = 4.0;
    sphere1.mat = redMaterial;

    Sphere sphere2;
    sphere2.position = float3(-12.0, -8.0, -30.0);
    sphere2.radius = 5.0;
    sphere2.mat = greenMaterial;

    Sphere sphere3;
    sphere3.position = float3(7.0, -3.0, -15.0);
    sphere3.radius = 3.5;
    sphere3.mat = blueMaterial;

    Sphere sphere4;
    sphere4.position = float3(-8.0, -6.0, -20.0);
    sphere4.radius = 2.5;
    sphere4.mat = yellowMaterial;

    Sphere sphere5;
    sphere5.position = float3(-5.0, -5.0, -10.0);
    sphere5.radius = 3.0;
    sphere5.mat = grayMaterial;

    Sphere sphere6;
    sphere6.position = float3(10.0, -5.0, -5.0);
    sphere6.radius = 2.0;
    sphere6.mat = purpleMaterial;

    Sphere sphere7;
    sphere7.position = float3(-3.0, -4.0, -18.0);
    sphere7.radius = 3.5;
    sphere7.mat = orangeMaterial;


    // Array of spheres in the scene
    return {
        floorSphere, sphere1, sphere2, sphere3,
        sphere4, sphere5, sphere6, sphere7
    };

}

struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
    float4 viewParameters : OUT_VIEW_PARMAS;
    float4x4 localToWorldMatrix : OUT_MODEL_MATRIX;
    float2 screenSize : OUT_SCREEN_SIZE;
};

struct Ray{
    float3 origin;
    float3 dir;
}

struct HitInfo {
    bool didHit;
    float dst;
    float3 hitPoint;
    float3 normal;
    RayTracingMaterial hitMaterial;
    __init() { didHit = false; dst = 40000; hitPoint = float3(-10); normal = float3(0, 0, 0);}
}

struct RayTracingMaterial {
    float3 color;
    float3 emissiveColour;
    float3 specularColour;
    float emissionStrength;
    float smoothenss;
    float smoothenssProbability;
    __init() { color = float3(0.0); emissiveColour = float3(0.0); emissionStrength = 0.0; smoothenss = 0.1; smoothenssProbability = 0.3; specularColour = float3(1.0);};
}

struct Sphere {
    float3 position;
    float radius;
    RayTracingMaterial mat;
    __init() {position = float3(0) ; radius = 1;}
}

float3 GatherEnvLight(Ray ray, float3 sunLightPosition, float sunFocus, float sunIntensity) {
    float3 SkyColorHorizon = float3(1.0, 1.0, 1.0);
    float3 SkyColorZenith = float3(0.7  , 0.7, 0.9);
    float3 GroundColor = float3(0.4, 0.4,0.4);

    float skyGradientT = pow(smoothstep(0, 0.4, ray.dir.y),0.35);
    float groundToSkyT = smoothstep(-0.01, 0, ray.dir.y);
    
    float3 skyGradient = lerp(SkyColorHorizon, SkyColorZenith, skyGradientT);
    float sun = pow(max(0, dot(ray.dir, normalize(sunLightPosition))), sunFocus) * sunIntensity;
    
    return lerp(GroundColor, skyGradient, groundToSkyT) + sun * (float)(groundToSkyT >= 1);
}


HitInfo RaySphere(Ray ray, float3 spherecenter, float radius)
{
    HitInfo hitInfo;
    float3 offsetRayOrigin = ray.origin - spherecenter;

    // each variable is a part of quadratic equation that is determining weather ray hit the sphere surface 
    // 
    float a = dot(ray.dir, ray.dir);
    float b = 2 * dot(offsetRayOrigin, ray.dir);
    float c = dot(offsetRayOrigin, offsetRayOrigin) - radius * radius;

    float discriminatn = b * b  - 4 * a * c;

    // hit hits the sphere at least once 
    if(discriminatn >= 0){

        // calculate the x1 of the quadratic equation to hae the front point and not the one in the back 
        float dst = (-b -sqrt(discriminatn)) / (2 * a);
        
        // ray hits sphere more than once 
        if(dst >= 0){
            hitInfo.didHit = true;
            hitInfo.dst = dst;
            // get the hit poinnt
            hitInfo.hitPoint = ray.origin + ray.dir * dst;
            hitInfo.normal = normalize(hitInfo.hitPoint - spherecenter);
        }
    }
    return hitInfo;
}

HitInfo CalculateRayCollision(Ray ray, Sphere spheres[], int sphereCount) {

    HitInfo closestHit;

    for (int i = 0; i < sphereCount; i++) {
        Sphere sphere = spheres[i];
        HitInfo hitInfo = RaySphere(ray, sphere.position, sphere.radius);

        if (hitInfo.didHit && hitInfo.dst < closestHit.dst){
            closestHit = hitInfo;
            closestHit.hitMaterial  = spheres[i].mat;
        }
    }
    return closestHit;
}

float RandomValue(inout uint state) {
    state = state * 747796405 + 2891336453;
    uint result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    result = (result >> 22) ^ result;
    return float(result) / 4294967295.0;
}

float RandomValueNormalDistribution(inout uint state) {
    float theta = 2 * 3.1415926 * RandomValue(state);
    float rho = sqrt(-2 * log(RandomValue(state)));
    return rho * cos(theta);
}

float3 RandomDirection(inout uint state) {

    float x = RandomValueNormalDistribution(state);
    float y = RandomValueNormalDistribution(state);
    float z = RandomValueNormalDistribution(state);
    return normalize(float3(x, y, z));
}

float3 RandomHemisphereDirection(float3 normal, inout uint rngState)
{
    float3 dir = RandomDirection(rngState);
    return dir * sign(dot(normal, dir));
}


float2 RandomPointInCircle(inout uint rngState){
    float angle = RandomValue(rngState) + 2 * PI;
    float2 pointOnCircle = float2(cos(angle), sin(angle));

    return pointOnCircle * sqrt(RandomValue(rngState));
}

float3 Trace(Ray ray,Sphere[] sceneData, float3 lightPos, int MaxBounceCount, inout uint rngState) {
    float3 rayColour = 1;
    float3 incomingLight = 0;
    for (int i = 0; i <= MaxBounceCount; i++)
    {
        HitInfo hitInfo = CalculateRayCollision(ray, sceneData, 9);
        if (hitInfo.didHit) {
            // if hit is detected set ray origing from camera to the hit point
            ray.origin = hitInfo.hitPoint;
            float3 diffuseDir = normalize(hitInfo.normal + RandomHemisphereDirection(hitInfo.normal, rngState));
            float3 specularDir = reflect(ray.dir, hitInfo.normal);
            
            bool isSpecular = hitInfo.hitMaterial.smoothenssProbability >= RandomValue(rngState);

            ray.dir = lerp(diffuseDir, specularDir, hitInfo.hitMaterial.smoothenss * isSpecular);

            // calcualte the newe colour of the ray (pixel)
            RayTracingMaterial hitSphereMaterial = hitInfo.hitMaterial;
            float3 emittedLight = hitSphereMaterial.emissiveColour * hitSphereMaterial.emissionStrength;
            incomingLight += emittedLight * rayColour;
            rayColour *= lerp(hitSphereMaterial.color, hitSphereMaterial.specularColour, isSpecular);
            //incomingLight = ray.dir;
        
        } else 
        {
            incomingLight += GatherEnvLight(ray, lightPos, 160.5, 100.8) * rayColour;
            break;
        }
    }

    return incomingLight;
}

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {

    float DivergeStrength = 10;
    float DefocuseStreanth = 2;

    uint2 numPixels = fsIn.screenSize;

    uint2 pixelCoord = (fsIn.uv) * numPixels;
    uint pixelIndex = pixelCoord.y * numPixels.x + pixelCoord.x;
    uint rngState = pixelIndex;

    float3 viewPointLocal = float3((fsIn.uv)- 0.5, 1) * fsIn.viewParameters.xyz;
    float4 viewPoint = mul(fsIn.localToWorldMatrix, float4(viewPointLocal,1));

    float3 camRight = fsIn.localToWorldMatrix._m00_m10_m20;
    float3 camUp = fsIn.localToWorldMatrix._m01_m11_m21;
    int raysPerPixel = 15;
    float3 totalIncomingLight = 0; 
    
    Ray ray;
    
    float2 defocuseJitter = RandomPointInCircle(rngState) * DefocuseStreanth/numPixels.x;
    ray.origin = fsIn.cameraPos;

    ray.dir = -normalize(viewPoint.xyz - ray.origin);
    //ray.dir.x *= -1.0;
    //ray.dir.y *= -1.0;

    Sphere scene[] = BuildScene();

    for (int rayIndex = 0; rayIndex < raysPerPixel; rayIndex++) {
        totalIncomingLight += Trace(ray,scene, fsIn.lightPos, 15, rngState);
    }

    float3 col = totalIncomingLight / raysPerPixel;

    return float4(col, 1);
}
