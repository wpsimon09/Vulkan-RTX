#define PI 3.14159265359

//=================================
// STRUCTS 
//=================================
struct SurfaceGeometry
{
    float3 position;
    float3 normal;
    float2 uv;
}

struct MaterialConstantValues
{
    float roughness;
    float metalness;
    float ao;
    float padding;

    float4 diffuse;

    float4 emissive_strength;
}

struct MaterialFeatures
{
    bool hasDiffuseTexture;
    bool hasEmissiveTexture;
    bool hasNormalTexture;
    bool hasArmTexture;
}

struct Material
{
    MaterialConstantValues Values;
    MaterialFeatures Features;
};


//===================================
// MATERIAL INTERFACE DEFINITIONS
//===================================

interface IMaterial
{
    associatedtype BRDF : IBRDF;

    BRDF Prepare(SurfaceGeometry geometry);
}

struct PBRMaterial : IMaterial
{

    MaterialFeatures features;
    MaterialConstantValues values;
    Sampler2D albedo_map;
    Sampler2D arm_map;
    Sampler2D normal_map;

    float3 albedo;
    float metallness;
    float roughness;
    float ao;

    typedef CookTorrence_GGX BRDF;

    CookTorrence_GGX Prepare(SurfaceGeometry geometry)
    {
        CookTorrence_GGX brdf;
        if (features.hasDiffuseTexture)
        {
            brdf.albedo = albedo_map.Sample(geometry.uv).rgb;
        }
        else
        {
            brdf.albedo = values.diffuse.rgb;
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
            // TODO: add emmisive textures
        }

        return brdf;
    }
}

//=========================================
// BRDF DEFINITIONS
//=========================================

interface IBRDF
{

    /**
     * wo -         incoming view direction
     * wi -         incoming light direction
     * N  -         normalVector;
     * float3 -     result colour
     */
    float3 Evaluate(float3 wo, float3 wi, float3 N);
}

struct CookTorrence_GGX : IBRDF
{

    float3 albedo;

    float roughness;

    float metallnes;

    float ao;

    float DistributionGGX(float3 N, float3 H)
    {
        float a = roughness * roughness;
        float apow = a * a;
        float NdotH = max(dot(N, H), 0.0);
        float NdothH2 = NdotH * NdotH;

        float num = apow;

        float denom = (NdothH2 * (apow - 1.0) + 1.0);
        float denonm = PI * denom * denom;

        return num / denom;
    }

    float GeometryShlickGGX(float NdotVorL)
    {
        float r = (roughness + 1.0);
        float k = (r * r) / 8;
        float num = NdotVorL;
        float deonm = NdotVorL * (1.0 - k) + k;

        return num / deonm;
    }

    float GeometrySmith(float3 N, float3 V, float3 L)
    {
        float NdotV = max(dot(N, V), 0.0);
        float ggx2 = GeometryShlickGGX(NdotV);

        float NdotL = max(dot(N, L), 0.0);
        float ggx1 = GeometryShlickGGX(NdotL);

        return ggx1 * ggx2;
    }

    float3 FresnelShlick(float cosThete, float3 F0)
    {
        return F0 + (1.0 - F0) * pow(1.0 - cosThete, 5.0);
    }

    float3 Evaluate(float3 V, float3 L, float3 N)
    {

        float3 F0 = float3(0.04);
        F0 = lerp(F0, albedo, metallnes);
        float3 H = normalize(V + L);

        float NDF = DistributionGGX(N, H);
        float G = GeometrySmith(N, V, L);
        float3 F = FresnelShlick(max(dot(H, V), 0.0), F0);

        float3 numerator = NDF * G * F;

        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.01);

        float3 specular = numerator / (max(denominator, 0.001));

        float3 ks = F;
        float3 kd = float3(1.0) - ks;

        kd *= 1.0 - metallnes;

        float NdotL = max(dot(N, L), 0);

        return (kd * albedo / PI + specular) * NdotL;
    }
}

//================================
// UTILITIES
//================================

float3 SampleNormalMap(Sampler2D normalMap, float2 UV, float3 N, float3 P)
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

    return normalize(mul(tangentNormal, TBN));
}

