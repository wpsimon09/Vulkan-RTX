#define PI 3.14159265359

struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
};

struct TextureSampler{
    Texture2D texture;
    SamplerState sampler;
    float4 Sample(float2 uv) {
        return texture.Sample(sampler, uv);
    }
}

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

    float DistributionGGX(float N, float H)  {
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

    float GeometryShlickGGX()

    float3 Evaluate(float3 V, float3 L, float3 N) {

        return 0;
    }
}

[[vk::binding(2,0)]]
ConstantBuffer<TextureSampler> albedo;

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {
    float4 fragColor;
    fragColor = albedo.Sample(fsIn.uv);

    return fragColor; 
}

