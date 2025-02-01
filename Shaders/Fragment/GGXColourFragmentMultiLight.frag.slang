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


[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn)
    : SV_Target
{
    DirectionalLight testDirLight;
    testDirLight.direction = normalize(fsIn.lightPos - fsIn.fragPos);
    testDirLight.intensity = float3(2.f);

    LightPair<DirectionalLight, EmptyLightEnv> lightingEnv;
    lightingEnv.first = testDirLight;

    EmptyLightEnv nolight;
    lightingEnv.second = nolight;

    SurfaceGeometry g;
    g.position = fsIn.fragPos;
    g.normal = normalize(fsIn.normal);
    g.uv = fsIn.uv;

    
    PBRMaterial material;
    material.features = _features;
    material.values = _materialNoTextureValues;
    material.albedo_map = _albedoMap;
    material.arm_map = _armMap;
    material.normal_map = _normalMap;
    let brdf_ = material.Prepare(g);

    if (material.features.hasNormalTexture) {
        g.normal = normalize(SampleNormalMap(_normalMap, fsIn.uv, fsIn.normal, fsIn.fragPos));
    }
  
    //CookTorrence_GGX brdf;
    float3 V = normalize(fsIn.cameraPos - fsIn.fragPos);
   
    float3 ambient = brdf_.albedo * 0.5;

    float3 finalColor = testDirLight.illuminate(g, brdf_, V) + ambient ;

    return float4(finalColor, 1.0);
}

