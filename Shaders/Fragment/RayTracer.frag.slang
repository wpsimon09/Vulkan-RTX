struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
    float4 viewParameters : OUT_VIEW_PARMAS;
    float4x4 localToWorldMatrix : OUT_MODEL_MATRIX;
};


struct MaterialConstantValues{
    float roughness;
    float metalness;
    float ao;
    float padding;

    float4 diffuse;

    float4 emissive_strength;
}

struct MaterialFeatures{
    bool hasDiffuseTexture;
    bool hasEmissiveTexture;
    bool hasNormalTexture;
    bool hasArmTexture;
}

struct Material{
    MaterialConstantValues Values;
    MaterialFeatures Features;
};

[[vk::binding(2, 0)]]
ConstantBuffer<MaterialConstantValues> _materialNoTextureValues;

[[vk::binding(3, 0)]]
ConstantBuffer<MaterialFeatures> _features;

[[vk::binding(4, 0)]]
ConstantBuffer<Sampler2D> _albedoMap;

[[vk::binding(5, 0)]]
ConstantBuffer<Sampler2D> _normalMap;

[[vk::binding(6, 0)]]
ConstantBuffer<Sampler2D> _armMap;

[[vk::binding(7, 0)]]
ConstantBuffer<Sampler2D> _emissiveMap;

struct Ray{
    float3 origin;
    float3 dir;
}

struct HitInfo {
    bool didHit;
    float dst;
    float3 hitPoint;
    float3 normal;
}

HitInfo RaySphere(Ray ray, float3 spherecenter, float radius)
{
    HitInfo hitInfo;
    hitInfo.didHit = false;
    float3 offsetRayOrigin = ray.origin - spherecenter;

    // each variable is a part of quadratic equation that is determining weather ray hit the sphere surface 
    float a = dot(ray.dir, ray.dir);
    float b = 2 * dot(offsetRayOrigin, ray.dir);
    float c = dot(offsetRayOrigin, offsetRayOrigin) - radius * radius;

    float discriminatn = b * b  - 4 * a * c;

    // hit hits the sphere at least once 
    if(discriminatn >= 0){
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

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {

    Material _materialDstSet;
    _materialDstSet.Features = _features;
    _materialDstSet.Values = _materialNoTextureValues;

    float3 viewPointLocal = float3((fsIn.uv)- 0.5, 1) * fsIn.viewParameters.xyz;
    float4 viewPoint = mul(fsIn.localToWorldMatrix, float4(viewPointLocal,1));

    Ray ray;
    ray.origin = fsIn.cameraPos;
    ray.dir = normalize(viewPoint.xyz - ray.origin);

    //return float4(normalize(viewPointLocal), 1.0);
    //return float4(float3(RaySphere(ray, 0, 1).didHit), 1.0);
    return float4(ray.dir,0.0);
}
