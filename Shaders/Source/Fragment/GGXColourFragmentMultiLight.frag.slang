import Material;
import Light;
import Surface;

struct FragmentShaderIn
{
    float3 normal : OUT_NORMAL; 
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float2 uv : OUT_UV;
    Material material : OUT_MATERIAL;
};

struct LightInfo {
    DirectionalLight dirLight;
    // PointLight[10] pointLigths;
    LightArray<PointLight, 20> pointLights;
    LightArray<AreaLight, 4> areaLights;
    float4 info;
}

[[vk::binding(2, 0)]]
ConstantBuffer<LightInfo> _lightInfo;

[[vk::binding(6, 0)]]
ConstantBuffer<Sampler2D> _albedoMap;

[[vk::binding(7, 0)]]
ConstantBuffer<Sampler2D> _normalMap;

[[vk::binding(8, 0)]]
ConstantBuffer<Sampler2D> _armMap;

[[vk::binding(9, 0)]]
ConstantBuffer<Sampler2D> _emissiveMap;

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn)
    : SV_Target
{
   SurfaceGeometry g;
    g.position = fsIn.fragPos;
    g.normal = normalize(fsIn.normal);
    g.uv = fsIn.uv;

    
    PBRMaterial material;
    material.features = fsIn.material.Features;
    material.values = fsIn.material.Values;
    material.albedo_map = _albedoMap;
    material.arm_map = _armMap;
    material.normal_map = _normalMap;
    material.emissive_map = _emissiveMap;
    let brdf_ = material.Prepare(g);

    if (material.features.hasNormalTexture) {
        g.normal = normalize(SampleNormalMap(_normalMap, fsIn.uv, fsIn.normal, fsIn.fragPos));
    }

    LightPair<LightPair<DirectionalLight, LightArray<PointLight, 20>>, LightPair<LightArray<AreaLight,4>,AmbientLight>> lightingEnv;
    LightPair<DirectionalLight, LightArray<PointLight, 20>> DirAndPoint;
    DirAndPoint.first = _lightInfo.dirLight;
    DirAndPoint.second = _lightInfo.pointLights;

    AmbientLight ambientLight;
    ambientLight.albedo = brdf_.albedo.xyz;
    ambientLight.aoIntensity = _lightInfo.info.y;
    ambientLight.roughness = brdf_.roughness;
    ambientLight.applyIBL = bool(_lightInfo.info.x);

    LightPair<LightArray<AreaLight, 4>, AmbientLight> AreaAndAmbient;
    AreaAndAmbient.first = _lightInfo.areaLights;
    AreaAndAmbient.second = ambientLight;

    lightingEnv.first = DirAndPoint;
    lightingEnv.second = AreaAndAmbient;

    //CookTorrence_GGX brdf;
    float3 V = normalize(fsIn.cameraPos - fsIn.fragPos);
   
    float3 finalColor = lightingEnv.illuminate(g, brdf_, V);

     return float4(finalColor, brdf_.albedo.w);
    //return float4(g.normal, 1.0);
}

