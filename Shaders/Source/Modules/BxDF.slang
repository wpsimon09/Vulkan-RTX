module bxdf;

import Surface;
import Utils;
import Surface;
import RandomValues;

public struct BxDFSample{
    float3 wi;
    float PDF;
}

public struct BxDFContext{
    float3 albedo;
    float metallnes;
    float roughness;
    float ao;
}

public interface IBRDF{
    public BxDFSample Sample(float3 wo, SurfaceGeometry g, inout uint u);
    public float3 Evaluate(float3 wo, float3 wi, SurfaceGeometry g);
    public float3 EvaluateByLTC(float3 wo, SurfaceGeometry g, float4 points, Sampler2D LTC, Sampler2D LTC_Inverse);
}


public struct Specular_GGX : IBRDF{

    BxDFContext context;
    /*
    Normal distribution 
    */
    float DistributionGGX(float3 N, float3 H)
    {
        float a = context.roughness * context.roughness;
        float apow = a * a;
        float NdotH = max(dot(N, H), 0.0);
        float NdothH2 = NdotH * NdotH;

        float num = apow;

        float denom = (NdothH2 * (apow - 1.0) + 1.0);
        float denonm = float.getPi() * denom * denom;

        return num / denom;
    }

    float3 FresnelSchlick(float cosTheta, float3 F0){
        return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    }

    float Smith_G1(float3 w1, float3 w2){
        float dotProduct = normalize(max(dot(w1, w2), 0.0));
        
        float r = (context.roughness + 1.0);
        float k = (r * r) / 8;
        float num = dotProduct;
        float deonm = dotProduct * (1.0 - k) + k;

        return num / deonm;
    }

    float GeometrySmith(float3 N, float3 V, float3 L){
        float ggx2 = Smith_G1(N , V);
        float ggx1 = Smith_G1(N, L);

        return ggx1 * ggx2;
    }

    public BxDFSample Sample(float3 wo, SurfaceGeometry g, inout uint u){

        // Transform outgoing/view direction from world to tangent space
        float3 wo_T = normalize(mul(g.iTBN, wo)); // view direction in tangent space

        // Compute stretched view direction (i only use issotropic case for now)
        float alpha = context.roughness * context.roughness;
        float3 Vh = normalize(float3(alpha * wo_T.x, alpha * wo_T.y, wo_T.z)); // stretched view

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
        float3 Ne = normalize(float3(alpha * Nh.x, alpha * Nh.y, max(0.0, Nh.z)));

        // Reflect view direction around sampled normal
        float3 wi_T = reflect(-wo_T, Ne); // light direction in tangent space

        // Transform sampled direction to world space

        float3 wi = normalize(mul(g.TBN, wi_T));

        float NdotV = max(0.0001, dot(Ne, wo_T));
        float3 H = normalize(Ne);          // microfacet normal (tangent space)
        float3 V = normalize(wo_T);        // view direction (tangent space)
        float3 L = normalize(wi_T);        // calculated light direction tangent space 
        
        BxDFSample sample;

        // TODO: this is accordint to the paper for the BRDF that is a bit differnet then my evaluation
        let D_h = DistributionGGX(g.normal, Ne);
        let G_1 = Smith_G1(Ne, wo); // TODO: maybe rewrite
        sample.PDF = (D_h * G_1) / 4 * NdotV;

        sample.wi = wi;

        return sample;
    } 
      
    public float3 Evaluate(float3 wo, float3 wi, SurfaceGeometry g);
    public float3 EvaluateByLTC(float3 wo, SurfaceGeometry g, float4 points, Sampler2D LTC, Sampler2D LTC_Inverse);
}
