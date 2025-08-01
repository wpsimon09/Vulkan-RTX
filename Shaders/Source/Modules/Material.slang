module Material;

import Utils;
import Surface;
import RandomValues;
#define PI 3.14159265359



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
    associatedtype BRDF : IBRDF;

    BRDF Prepare(SurfaceGeometry geometry);
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
    public float4 emission;
    
    public float metallness;
    public float roughness;
    public float ao;
    
    public typedef CookTorrence_GGX BRDF;
    
    public CookTorrence_GGX Prepare(SurfaceGeometry geometry)
    {
        CookTorrence_GGX brdf;
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
            brdf.emission = emissive;
            if (brdf.emission.w == 0.0) {
                brdf.emission.w = values.emissive_strength.w;
            }
        } else {
            brdf.emission = values.emissive_strength;
        }

        return brdf;
    }
}

//=========================================
// BRDF DEFINITIONS
//=========================================

public interface IBRDF
{

    float3 FresnellShlickRoughness(float cosTheta);

    /**
     * wo -         incoming view direction
     * wi -         incoming light direction
     * N  -         normalVector;
     * float3 -     result colour
     */
    float3 Evaluate(float3 wo, float3 wi, float3 N);

    /**
     * wo -         incoming view direction
     * wi -         incoming light direction
     * N  -         normalVector;
     * float3 -     result colour
     */
    float3 EvaluateSpecular(float3 wo, float3 wi, float3 N, float PDF);

    /**
     * wo -         incoming view direction
     * wi -         incoming light direction
     * N  -         normalVector;
     * float3 -     result colour
     */
    float3 EvaluateDiffuse(float3 wo, float3 wi, float3 N, float PDF);

    /**
     * wo -         incoming view direction
     * wi -         incoming light direction
     * N  -         normalVector;
     * float3 -     result colour
     */
    float3 EvaluateByLTC(float3 wo, float3 N, float3 P,float4 points[4], bool twoSided, Sampler2D LTC,Sampler2D LTC_Inverse);

    /**
    * wo -             outgoing (view) direction in WORLD space
    * g -              surface that is being shaded 
    * out PDF -        probability density function of the returned sample
    * randomContext - random context for sample points u1 and u2 generations
    */
   float3 SampleGGX_VND(float3 wo, SurfaceGeometry g, out float PDF, inout uint randomContext);
   
   /**
    * wo -             outgoing (view) direction in WORLD space
    * g -              surface that is being shaded 
    * out PDF -        probability density function of the returned sample
    * randomContext - random context for sample points u1 and u2 generations
    */
    float3 SampleLambertianDiffuse(float3 wo, SurfaceGeometry g ,out float PDF, inout uint randomContex);

}

public struct CookTorrence_GGX : IBRDF
{

    public float4 albedo;

    public float roughness;

    public float metallnes;

    public float ao;

    public float4 emission;

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

    float D_v(float3 wo, float3 N_i, SurfaceGeometry g){
        float NiDotV = max(dot(N_i, wo), 0.0);
        float NdotV  = max(dot(g.shadingNormal, wo), 0.0);
        float G_1 = GometryShlickGGXHeitz(NdotV);
        float D = DistributionGGX(g.shadingNormal, N_i);

        return (G_1 * NiDotV * D) / max(NdotV ,1e-5);
    }

    float GometryShlickGGXHeitz(float NdotV){
        float alpha = roughness * roughness;
        float k = alpha * 0.5;
        float denom = NdotV* (1.0 - k ) + k;
        return NdotV / denom;
    }

    float GeometryShlickGGXUnreal(float NdotVorL)
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
        float ggx2 = GeometryShlickGGXUnreal(NdotV);

        float NdotL = max(dot(N, L), 0.0);
        float ggx1 = GeometryShlickGGXUnreal(NdotL);

        return ggx1 * ggx2;
    }

    float3 FresnelShlick(float cosThete, float3 F0)
    {
        return F0 + (1.0 - F0) * pow(1.0 - cosThete, 5.0);
    }

    public float3 FresnellShlickRoughness(float cosTheta)
    {
        float3 F0 = float3(0.04);
        F0 = lerp(F0, albedo.xyz, metallnes);
        return F0 + (max(float3(1.0 - roughness), F0) - F0) *
                        pow(1.0 - cosTheta, 5.0);
    }

    public float3 Evaluate(float3 V, float3 L, float3  N)
    {

        float3 F0 = float3(0.04);
        F0 = lerp(F0, albedo.xyz, metallnes);
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

        float3 colour =  (kd * albedo.xyz / PI + specular) * NdotL;

        //colour += (this.emission.xyz * this.emission.w);

        return colour;
    }

    
    public float3 SampleGGX_VND(float3 wo, SurfaceGeometry g, out float PDF, inout uint randomContext){       
        //***********************************************************************************************************************************
        // this is unintuitive but it physically correct, the wo is outgoing direciton which is the ray that hit the surface which normal
        // we are trying tosample, wi is incident direction which is the ray that left the surface and is going somwhere in the scene
        // in real world we would gather all rays coming from the scene to the eye but that would kill your computer 
        //***********************************************************************************************************************************

        // Transform outgoing/view direction from world to tangent space
        float3 wo_T = normalize(mul(g.iTBN, wo)); // view direction in tangent space

        // Compute stretched view direction (i only use issotropic case for now)
        float alpha = roughness * roughness;
        float3 Vh = normalize(float3(alpha * wo_T.x, alpha * wo_T.y, wo_T.z)); // stretched view

        // Orthonormal basis around Vh 
        float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
        float3 T1 = lensq > 0.0 ? float3(-Vh.y, Vh.x, 0.0) * inverseSqrt(lensq) : float3(1.0, 0.0, 0.0);
        float3 T2 = cross(Vh, T1);

        // Sample a point on projected hemisphere
        float u1 = RandomValue(randomContext);
        float u2 = RandomValue(randomContext);

        float r = sqrt(u1);
        float phi = 2.0 * PI * u2;
        float t1 = r * cos(phi);
        float t2 = r * sin(phi);
        float s = 0.5 * (1.0 + Vh.z);
        t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

        float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

        // Transform Nh back to the hemisphere configuration
        float3 Ne = normalize(float3(alpha * Nh.x, alpha * Nh.y, max(0.0, Nh.z)));

        // Reflect view direction around sampled normal
        float3 wi_T = reflect(-wo_T, Ne); // light direction in tangent space

        // Transform sampled direction to world space
        float3 wi = normalize(mul(g.TBN, wi_T));

        PDF = D_v(wo, wi, g) / (max(4 * (dot(wo, wi)), 1e-5));

        return wi;
    }

    public float3 EvaluateSpecular(float3 wo, float3 wi, float3 N, float PDF ){
        float3 F0 = float3(0.04);
        F0 = lerp(F0, albedo.xyz, metallnes);
        float3 H = normalize(wo + wi);

        float NDF = DistributionGGX(N, H);
        float G = GeometrySmith(N, wo, wi);
        float3 F = FresnelShlick(max(dot(H, wo), 0.0), F0);

        float3 numerator = NDF * G * F;

        float denominator = 4.0 * max(dot(N, wo), 0.0) * max(dot(N, wi), 0.01);

        float3 specular = numerator / (max(denominator, 0.001));

        return specular;
    }

    public float3 EvaluateDiffuse(float3 wo, float3 wi, float3 N, float PDF){

        float cosTheta = max(dot(N, wo), 0);
        return albedo.xyz * cosTheta;
    }

    public float3 SampleLambertianDiffuse(float3 wo, SurfaceGeometry g ,out float PDF, inout uint randomContex){

        float u1 = RandomValue(randomContex);
        float u2 = RandomValue(randomContex);

        float r = sqrt(u1);
        float theta = 2.0f * float.getPi() * u2;

        float x = r * cos(theta);
        float y = r * sin(theta);
        float z = sqrt(1.0f - u1);  // this ensures cosine-weighting

        // cosine weighted tangent hemisphere vector
        float3 dir = float3(x, y, z);

        // go from tangent space to the world space for cosine calculation
        float3 dir_Ws = mul(g.TBN , dir);

        float cosTheta = max(0.0, dot(g.shadingNormal, dir_Ws));

        PDF =  cosTheta / float.getPi();
        
        return dir_Ws;
    }

    public float3 EvaluateByLTC(float3 wo,float3 N , float3 P, float4 points[4], bool twoSided, Sampler2D LTC, Sampler2D LTC_Inverse) {

        const float LUT_SIZE = 64.0; // ltc_texture size
        const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
        const float LUT_BIAS = 0.5 / LUT_SIZE;

        float NdotV = max(dot(N, wo),0.f);
        float2 uv = float2(roughness, 1.0-sqrt(1.0f - NdotV));
        uv = uv * LUT_SCALE + LUT_BIAS;

        float4 t1 = LTC_Inverse.Sample(float2(uv.x, 1.0 - uv.y));
        float4 t2 = LTC.Sample(float2(uv.x, 1.0 - uv.y));

        float3 F0 = float3(0.04);
        F0 = lerp(F0, albedo.xyz, metallnes);
        float3 F = FresnelShlick(max(dot(N, wo), 0.0), F0);

        float3 kS = F;
        float3 kD = float3(1.0) - kS;
        kD *= 1.0 - metallnes;
        
        float3x3 MinV;
        MinV[0] = float3(t1.x, 0.0,   t1.y);
        MinV[1] = float3(0.0, 1.0 ,   0.0);
        MinV[2] = float3(t1.z,   0.0,  t1.w);

        float3x3 identity;
        identity[0] = float3(1.0, 0.0, 0.0);
        identity[1] = float3(0.0, 1.0, 0.0);
        identity[2] = float3(0.0, 0.0, 1.0);

        float3 specular = CalculateAreaLightLo(normalize(wo), P, normalize(N), MinV, points, twoSided, LTC, LTC_Inverse);
        float3 diffuse = CalculateAreaLightLo(normalize(wo), P, normalize(N), identity, points, twoSided, LTC, LTC_Inverse);

        specular *= albedo.xyz * t2.x + (1.0f - albedo.xyz) * t2.y;

        float3 colour =  (specular + (diffuse * albedo.xyz));

        colour += (this.emission.xyz * this.emission.w);

        return colour;
    }

    
    public float3 EvaluateForShadow(){
        return albedo.xyz * 0.4;
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
