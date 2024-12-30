#define PI 3.14159265359

struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
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

    [mutating]
    void FillWithData(Material mat, float2 uv) {
        if (mat.Features.hasDiffuseTexture) {
            this.albedo = _albedoMap.Sample(uv).rgb;
        } else { 
            this.albedo = mat.Values.diffuse.rgb;
        }

        if (mat.Features.hasArmTexture) {
            float3 armValues = _armMap.Sample(uv).rgb;
            this.ao =        armValues.r;
            this.roughness = armValues.g;
            this.metallnes = armValues.b;
        } else {
            this.roughness = mat.Values.roughness;
            this.metallnes = mat.Values.metalness;
            this.ao =        mat.Values.ao;
        }

        if (mat.Features.hasEmissiveTexture) {
            //TODO: add suport for emmision
        }
    }

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

        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.01);

        float3 specular = numerator/(max(denominator, 0.001));

        float3 ks = F;
        float3 kd = float3(1.0) - ks;

        kd *= 1.0 - metallnes;

        float NdotL = max(dot(N, L), 0);

        return (kd * albedo / PI + specular) * NdotL;
    }

}

float3 SampleNormalMap(Sampler2D normalMap, float2 UV, float3 N ,float3 P)
{
    float3 tangentNormal = normalMap.Sample(UV).xyz * 2.0 - 1.0;

    float3 Q1  = ddx(P);
    float3 Q2  = ddy(P);
    
    float2 st1 = ddx(UV);            
    float2 st2 = ddy(UV);

    float3 N   = normalize(N);
    float3 T  = normalize(Q1*st2.y - Q2*st1.y);
    float3 B  = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    return normalize(mul(tangentNormal, TBN));
}


[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {

    Material _materialDstSet;
    _materialDstSet.Features = _features;
    _materialDstSet.Values = _materialNoTextureValues;

    float3 lightColor = float3(2.0);
    
    CookTorrence_GGX brdf;
    brdf.FillWithData(_materialDstSet, fsIn.uv);
    
    float3 V = normalize(fsIn.cameraPos - fsIn.fragPos);

    float3 N;
    if (_materialDstSet.Features.hasNormalTexture) {
        N = normalize(SampleNormalMap(_normalMap, fsIn.uv, fsIn.normal, fsIn.fragPos));
    } else {
        N = normalize(fsIn.normal);
    }

    float3 L = normalize(fsIn.lightPos - fsIn.fragPos);

    float3 color = brdf.Evaluate(V, L, N) * lightColor;

    float3 ambient = brdf.albedo * 0.8;

    float3 finalColor = max(color,float(0.0)) + ambient;

    return float4(finalColor, 1.0);
}   

