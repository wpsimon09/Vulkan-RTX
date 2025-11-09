module bxdf;

import Surface;
import Utils;
import Surface;
import RandomValues;

public static const float ONE_OVER_PI = 1/float.getPi();


public struct BxDFSample{
    public float3 wi;
    public float PDF;
}

public struct BxDFContext{
    public float4 albedo;
    public float3 emmision;
    public float emissionStrength;
    public float metallnes;
    public float roughness;
    public float ao;

    internal float alpha;
}

public interface IBRDF{
    public Optional<BxDFSample> Sample(float3 wo, SurfaceGeometry g, inout uint u);
    public float3 Evaluate(float3 wo, float3 wi, SurfaceGeometry g);
    public float3 EvaluateByLTC(float3 wo, SurfaceGeometry g, float4 points, Sampler2D LTC, Sampler2D LTC_Inverse);
    public BxDFContext GetContext();
}


public struct Specular_GGX : IBRDF{
    
    BxDFContext context;
    public float3 F0;

    public __init(BxDFContext context){
        this.context = context;
        this.context.alpha = context.roughness * context.roughness;
        
        F0 = lerp(context.albedo.xyz, 0.4, context.metallnes);
    }
    /*
    Normal distribution 
    */
    float DistributionGGX(float3 N, float3 H)
    {
        float NdotH = max(0.0, dot(N, H));
        float alphaSquared = context.alpha * context.alpha;

        float b = ((alphaSquared - 1.0f) * NdotH * NdotH + 1.0f);
        return alphaSquared / (PI * b * b);
    }

    public float3 FresnelSchlick(float cosTheta, float3 F0){
        return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    }

    float Smith_G1(float3 w1, float3 w2){
        float alpha = context.alpha;
        float alphaSqared = alpha * alpha;

        float NdotS = normalize(max(dot(w1, w2), 0.0));
        float NdotSSquared = NdotS * NdotS;
        
         return 2.0f / (sqrt(((alphaSqared * (1.0f - NdotSSquared)) + NdotSSquared) /
                      NdotSSquared) +
                 1.0f);
    }

    float GeometrySmith(float3 N, float3 V, float3 L){
        float ggx2 = Smith_G1(N , V);
        float ggx1 = Smith_G1(N, L);

        return ggx1 * ggx2;
    }

    public Optional<BxDFSample> Sample(float3 wo, SurfaceGeometry g, inout uint u){

        float4 qRotationToZ = getRotationToZAxis(g.shadingNormal);
        float3 Ve = rotatePoint(qRotationToZ, wo);
        float2 alpha2D = float2(context.alpha, context.alpha);

        // Compute stretched view direction (i only use issotropic case for now)
        float3 Vh = normalize(float3(alpha2D.x * Ve.x, alpha2D.y * Ve.y, Ve.z)); // stretched view

        // Orthonormal basis around Vh 
        float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
        float3 T1 = lensq > 0.0 ? float3(-Vh.y, Vh.x, 0.0) * inverseSqrt(lensq) : float3(1.0, 0.0, 0.0);
        float3 T2 = cross(Vh, T1);

        // Sample a point on projected hemisphere
        float u1 = RandomValue(u);
        float u2 = RandomValue(u);

        float r = sqrt(u1);
        float phi = 2.0 * PI * u2;
        float t1 = r * cos(phi);
        float t2 = r * sin(phi);
        float s = 0.5 * (1.0 + Vh.z);
        t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

        float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

        // Transform Nh back to the hemisphere configuration

        // half way vector 
        float3 Ne = normalize(float3(alpha2D.x * Nh.x, alpha2D.y * Nh.y, max(0.0, Nh.z)));

        // local light direction 
        float3 Li = reflect(-Ve, Ne);

        float3 Li_world = normalize(rotatePoint(invertRotation(qRotationToZ), Li));

        BxDFSample sample;
  
        const float3 Nlocal = float3(0.0f, 0.0f, 1.0f);
        float NdotV = max(0.0001f, dot(Nlocal, Ve));

        let D_h = DistributionGGX(Nlocal, Ne);
        let G_1 = Smith_G1(Ve, Nlocal);

        sample.PDF = (D_h * G_1) / (4 * NdotV);

        sample.wi = Li_world;

        return sample;
    } 
      
    public float3 Evaluate(float3 wo, float3 wi, SurfaceGeometry g)
    {
        if(dot(g.shadingNormal, wo) <= 0.0f){
                return float3(0.0f);
        }   

        float3 F0 = float3(0.04);
        F0 = lerp(F0, context.albedo.xyz, context.metallnes);
        float3 H = normalize(wo + wi);

        float NDF = DistributionGGX(g.shadingNormal, H);
        float G = GeometrySmith(g.shadingNormal, wo, wi);
        float3 F = FresnelSchlick(max(dot(H, wo), 0.0), F0);

        float3 numerator = NDF * G * F;

        float denominator = 4.0 * max(dot(g.shadingNormal, wo), 0.0) * max(dot(g.shadingNormal, wi), 0.01);

        float3 col = numerator / (max(denominator, 0.001));

        return col;
    }

    public float3 EvaluateByLTC(float3 wo, SurfaceGeometry g, float4 points, Sampler2D LTC, Sampler2D LTC_Inverse){
        return context.albedo.xyz;
    }

    public BxDFContext GetContext(){
        return this.context;
    }
}


public struct LamberianDiffuse : IBRDF{

    public __init(BxDFContext context){
        this.context = context;
    }

    BxDFContext context;

    public Optional<BxDFSample> Sample(float3 wo, SurfaceGeometry g, inout uint u){
        float2 u2;
        u2.x = RandomValue(u);
        u2.y = RandomValue(u);
    

        float a = sqrt(u2.x);
        float b = 2 * float.getPi() * u2.y;

        float3 result = float3(
            a * cos(b),
            a * sin(b),
            sqrt(1.0f - u2.x)
        );

        // go from tangent space to the world space for cosine calculation
        float3 dir_Ws = mul(g.TBN , result);

        BxDFSample sample;

        sample.PDF =  result.z * (1.0 / float.getPi());
        sample.wi  = dir_Ws;

        return sample;
    }
    public float3 Evaluate(float3 wo, float3 wi, SurfaceGeometry g){
        if(dot(g.shadingNormal, wo) <= 0.0f){
            return float3(0.0f);
        }
        float3 diffuseReflectance = context.albedo.xyz * (1.0 - context.metallnes);
        float cosTheta = max(dot(g.shadingNormal, wo), 0.001);
        return diffuseReflectance * ONE_OVER_PI;
    }

    public float3 EvaluateByLTC(float3 wo, SurfaceGeometry g, float4 points, Sampler2D LTC, Sampler2D LTC_Inverse){
        return context.albedo.xyz;
    }

    public BxDFContext GetContext(){
        return this.context;
    }
}
 
/*
Used for rasterisation and combines both lambertian and specular brdf
*/
public struct DeterministicBRDF : IBRDF{
    LamberianDiffuse diffuseBrdf;
    Specular_GGX specularBrdf;

    BxDFContext context;

    public __init(BxDFContext context){
        this.specularBrdf = Specular_GGX(context);
        this.diffuseBrdf = LamberianDiffuse(context);
        this.context = context;
    };

    public Optional<BxDFSample> Sample(float3 wo, SurfaceGeometry g, inout uint u){
        return {};
    }
    
    public float3 Evaluate(float3 wo, float3 wi, SurfaceGeometry g){
        float3 diffuse = diffuseBrdf.Evaluate(wo, wi, g);
        float3 specular = specularBrdf.Evaluate(wo, wi, g);

        float3 kS = specularBrdf.FresnelSchlick(dot(wo, wi), specularBrdf.F0);
        float3 kD = float3(1.0) - kS;

        kD *= 1.0 -  context.metallnes;
        
        return ((diffuse) + (specular));
    }
    
    public float3 EvaluateByLTC(float3 wo, SurfaceGeometry g, float4 points, Sampler2D LTC, Sampler2D LTC_Inverse){
        return float3(1.0f);
    }

    public BxDFContext GetContext(){
        return this.context;
    }

}