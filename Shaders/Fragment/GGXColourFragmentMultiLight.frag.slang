import Material;
import Light;

struct FragmentShaderIn
{
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
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

[[vk::binding(8, 0)]]
ConstantBuffer<LightArray<DirectionalLight,1>> _directionalLight;

[[vk::binding(9, 0)]]
ConstantBuffer<LightArray<PointLight,100>> pointLights;


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


[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn)
    : SV_Target
{
    DirectionalLight testDirLight;
    testDirLight.direction = float3(45.f);
    testDirLight.intensity = float3(2.f);

    LightPair<DirectionalLight, PointLight> lightingEnv;
    lightingEnv.first = testDirLight;

    SurfaceGeometry g;
    g.position = fsIn.fragPos;
    g.normal = fsIn.normal;
    g.uv = fsIn.uv;

    
    PBRMaterial material;
    material.features = _features;
    material.values = _materialNoTextureValues;
    material.albedo_map = _albedoMap;
    material.arm_map = _armMap;
    let brdf_ = material.Prepare(g);

    if (material.features.hasNormalTexture) {
        g.normal = normalize(SampleNormalMap(_normalMap, fsIn.uv, fsIn.normal, fsIn.fragPos));
    }
    
    Material _materialDstSet;
    _materialDstSet.Features = _features;
    _materialDstSet.Values = _materialNoTextureValues;
    
    //CookTorrence_GGX brdf;

    float3 V = normalize(fsIn.cameraPos - fsIn.fragPos);

    float3 N;
    if (_materialDstSet.Features.hasNormalTexture)
    {
        N = normalize(SampleNormalMap(_normalMap, fsIn.uv, fsIn.normal, fsIn.fragPos));
    }
    else
    {
        N = normalize(fsIn.normal);
    }

    float3 L = normalize(fsIn.lightPos - fsIn.fragPos);

   
    float3 ambient = brdf_.albedo * 0.8;

    float3 finalColor = testDirLight.illuminate(g, brdf_, V) + ambient;

    return float4(finalColor, 1.0);
}

