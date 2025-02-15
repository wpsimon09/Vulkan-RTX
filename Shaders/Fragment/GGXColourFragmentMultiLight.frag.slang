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

struct LightInfo {
    DirectionalLight dirLight;
    // PointLight[10] pointLigths;
    LightArray<PointLight, 20> pointLights;
    LightArray<AreaLight, 4> areaLights;
}

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
ConstantBuffer<LightInfo> _lightInfo;

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn)
    : SV_Target
{
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

    // LightArray<PointLight, 10> pointLights;
    // pointLights.count = 10;
    // for (int i = 0; i < 10; i++) {
    //     pointLights.lights[i].position = _lightInfo.pointLigths[i].position;
    //     pointLights.lights[i].intensity = _lightInfo.pointLigths[i].intensity;
    // }

    if (material.features.hasNormalTexture) {
        g.normal = normalize(SampleNormalMap(_normalMap, fsIn.uv, fsIn.normal, fsIn.fragPos));
    }

    LightPair<LightPair<DirectionalLight, LightArray<PointLight, 20>>, LightPair<LightArray<AreaLight,4>,AmbientLight>> lightingEnv;
    LightPair<DirectionalLight, LightArray<PointLight, 20>> DirAndPoint;
    DirAndPoint.first = _lightInfo.dirLight;
    DirAndPoint.second = _lightInfo.pointLights;

    AmbientLight ambientLight;
    ambientLight.albedo = brdf_.albedo;
    ambientLight.aoIntensity = 0.1;

    LightPair<LightArray<AreaLight, 4>, AmbientLight> AreaAndAmbient;
    AreaAndAmbient.first = _lightInfo.areaLights;
    AreaAndAmbient.second = ambientLight;

    lightingEnv.first = DirAndPoint;
    lightingEnv.second = AreaAndAmbient;

    //CookTorrence_GGX brdf;
    float3 V = normalize(fsIn.cameraPos - fsIn.fragPos);
   
    float3 finalColor = lightingEnv.illuminate(g, brdf_, V);

    return float4(finalColor,1.0);
}

