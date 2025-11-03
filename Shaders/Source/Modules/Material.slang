module Material;

import Utils;
import Surface;
import RandomValues;
import BxDF;

public static const float ONE_OVER_PI = 1/float.getPi();

public struct MaterialConstantValues
{
    public float roughness;
    public float metalness;
    public float ao;
    public float padding;

    public float4 diffuse;

    public float4 emissive_strength;
}

public struct MaterialFeatures
{
    public bool hasDiffuseTexture;
    public bool hasEmissiveTexture;
    public bool hasNormalTexture;
    public bool hasArmTexture;

    public int albedoTextureIdx;
    public int armTextureIdx;
    public int emissiveTextureIdx;
    public int normalTextureIdx;
}

public struct Material
{
    public MaterialConstantValues Values;
    public MaterialFeatures Features;
};

//===================================
// MATERIAL INTERFACE DEFINITIONS
//===================================

public interface IMaterial
{
    BxDFContext Prepare(SurfaceGeometry geometry);
}

public struct PBRMaterial : IMaterial
{

    public MaterialFeatures features;
    public MaterialConstantValues values;
    
    public Sampler2D albedo_map;
    public Sampler2D arm_map;
    public Sampler2D normal_map;
    public Sampler2D emissive_map;
    
    public float4 albedo;
    public float3 diffuseReflectance;
    public float4 emission;
    
    public float metallness;
    public float roughness;
    public float ao;
    
    public typedef IBRDF BRDF;
    
    public BxDFContext Prepare(SurfaceGeometry geometry)
    {
        BxDFContext brdf;
        if (features.hasDiffuseTexture)
        {
            brdf.albedo = albedo_map.Sample(geometry.uv);
        }
        else
        {
            brdf.albedo = values.diffuse ;
        }

        if (features.hasArmTexture)
        {
            float3 armValues = arm_map.Sample(geometry.uv).rgb;
            brdf.ao = armValues.r;
            brdf.roughness = armValues.g;
            brdf.metallnes = armValues.b;
        }
        else
        {
            brdf.roughness = values.roughness;
            brdf.metallnes = values.metalness;
            brdf.ao = values.ao;
        }

        if (features.hasEmissiveTexture) {
            float4 emissive = emissive_map.Sample(geometry.uv);
            brdf.emmision = emissive.xyz;
            brdf.emissionStrength = values.emissive_strength.w;
        } else {
            brdf.emmision = values.emissive_strength.xyz;
            brdf.emissionStrength = values.emissive_strength.w;
        }
        return brdf;
    }
}


//================================
// UTILITIES
//================================

public float3 SampleNormalMap(Sampler2D normalMap, float2 UV, float3 N, float3 P)
{
    float3 tangentNormal = normalMap.Sample(UV).xyz * 2.0 - 1.0;

    float3 Q1 = ddx(P);
    float3 Q2 = ddy(P);

    float2 st1 = ddx(UV);
    float2 st2 = ddy(UV);

    float3 N = normalize(N);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    return normalize(mul(TBN, tangentNormal));
}

public float3 SampleNormalMap(Sampler2D normalMap,float2 UV, float3x3 TBN){
    float3 tangentNormal = normalMap.Sample(UV).xyz * 2.0 - 1.0;
    return normalize(mul(TBN, tangentNormal));
}

public float3 IntegrateEdgeVec(float3 v1, float3 v2) {
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5 * inverseSqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * theta_sintheta;
}

public float3 CalculateAreaLightLo(float3 wo, float3 P, float3 N, float3x3 MinV, float4 points[4], bool twoSided, Sampler2D LTC, Sampler2D LTC_Inverse

) {
    const float LUT_SIZE = 64.0; // ltc_texture size
    const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
    const float LUT_BIAS = 0.5 / LUT_SIZE;

    float3 T1, T2;
    T1 = normalize(wo - N * dot(wo, N));
    T2 = cross(N, T1);

    float3x3 TTN = float3x3(T1, T2, N);
    float3x3 minv =  mul(transpose(TTN),MinV);

    float3 L[4];
    L[0] = mul((points[0].xyz - P),minv);
    L[1] = mul((points[1].xyz - P),minv);
    L[2] = mul((points[2].xyz - P),minv);
    L[3] = mul((points[3].xyz - P),minv);

    float3 dir = points[0].xyz - P;

    float3 lightNormal = normalize(cross(points[1].xyz - points[0].xyz, points[3].xyz - points[0].xyz));
    bool behind = (dot(dir, lightNormal) < 0.0);

    // cos weighted space
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    // integrate
    float3 vsum = float3(0.0f);
    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]); 
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    float len = length(vsum);

    float z = vsum.z / len;
    if (behind) {
        z = -z;
    }

    float2 uv = float2(z * 0.5f + 0.5f, len);
    uv = uv * LUT_SCALE + LUT_BIAS;

    float scale = LTC.Sample(float2(uv.x, 1.0 - uv.y)).w;

    float sum = len * scale;

    if (!behind && !twoSided) {
        sum = 0.0f;
    }

    return float3(sum, sum, sum);
}
