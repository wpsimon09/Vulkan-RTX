#define PI 3.14159265359

struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
};




interface IBRDF {

    /**
     * wo -         incoming view direction
     * wi -         incoming light direction
     * N  -         normalVector;
     * float3 -     result colour
     */
    float3 Evaluate(float3 wo, float3 wi, float3 N);
}

struct CookTorrence_GGX: IBRDF {

    float3 albedo;
    float roughness;
    float metallnes;
    float ao;

    float DistributionGGX(float3 N, float3 H)  {
        float a = roughness * roughness;
        float apow = a* a;
        float NdotH = max(dot(N, H), 0.0);
        float NdothH2 = NdotH * NdotH;

        float num = apow;

        float denom = (NdothH2 * (apow - 1.0) + 1.0);
        float denonm = PI * denom * denom;

        return num / denom;
    }

    float GeometryShlickGGX(float NdotVorL) {
        float r = (roughness + 1.0);
        float k = (r * r) / 8;
        float num = NdotVorL;
        float deonm = NdotVorL * (1.0 - k) + k;

        return num / deonm;
    }

    float GeometrySmith(float3 N, float3 V, float3 L) {
        float NdotV = max(dot(N, V), 0.0);
        float ggx2 = GeometryShlickGGX(NdotV);

        float NdotL = max(dot(N, L), 0.0);
        float ggx1 = GeometryShlickGGX(NdotL);

        return ggx1 * ggx2;
    }

    float3 FresnelShlick(float cosThete, float3 F0) {
        return F0 + (1.0 - F0) * pow(1.0 - cosThete, 5.0);
    }

    float3 Evaluate(float3 V, float3 L, float3 N) {

        float3 F0 = float3(0.04);
        F0 = lerp(F0, albedo, metallnes);
        float3 H = normalize(V + L);

        float NDF = DistributionGGX(N, H);
        float G = GeometrySmith(N, V, L);
        float3 F = FresnelShlick(max(dot(H, V), 0.0), F0);

        float3 numerator = NDF * G * F;

        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);

        float3 specular = numerator/(max(denominator, 0.001));

        float3 ks = F;
        float3 kd = float3(1.0) - ks;

        kd *= 1.0 - metallnes;

        float NdotL = max(dot(N, L), 0.000001);

        return (kd * albedo / PI + specular) * NdotL;
    }
}

[[vk::binding(2,0)]]
ConstantBuffer<Sampler2D> albedo;

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {

    float3 lightColor = float3(10.0);

    float4 fragColor;
    fragColor = albedo.Sample(fsIn.uv);

    CookTorrence_GGX brdf;
    brdf.albedo = fragColor.xyz;
    brdf.roughness = 0.4;
    brdf.metallnes = 0.1;
    brdf.ao = 1;

    float3 V = normalize(fsIn.cameraPos - fsIn.fragPos);
    float3 N = normalize(fsIn.normal);

    float3 L = normalize(fsIn.lightPos - fsIn.fragPos);

    float3 color = brdf.Evaluate(V, L, N) * lightColor;

    float3 ambient = brdf.albedo * brdf.ao * 0.8 * max(dot(N, float3(0, 1, 0)), 0.2);;

    float3 finalColor = max(color,float(0.0)) + ambient;

    return float4(finalColor, 1.0);
}   

