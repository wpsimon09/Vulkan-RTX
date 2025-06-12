import Material;
import Light;
import Surface;

#include "ShaderBindingGroups/ForwardLit.slang"

struct FragmentShaderIn
{
    float3 normal : OUT_NORMAL; 
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float2 uv : OUT_UV;
    int materialIdx : OUT_MATERIAL;
    float2 screenSpaceUv: OUT_SS_UV;
};


[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn)
    : SV_Target
{
   SurfaceGeometry g;
    g.position = fsIn.fragPos;
    g.shadingPosition = fsIn.fragPos;
    g.shadingNormal = normalize(fsIn.normal);
    g.normal = normalize(fsIn.normal);
    g.uv = fsIn.uv;

    Material mat = materialValues[fsIn.materialIdx];

    PBRMaterial material;
    material.features = mat.Features;
    material.values = mat.Values;
    material.albedo_map = textures[mat.Features.albedoTextureIdx];
    material.arm_map = textures[mat.Features.armTextureIdx];
    material.normal_map = textures[mat.Features.normalTextureIdx];
    material.emissive_map = textures[mat.Features.emissiveTextureIdx];
    let brdf_ = material.Prepare(g);

    if (material.features.hasNormalTexture) {
        g.normal = normalize(SampleNormalMap(material.normal_map, fsIn.uv, fsIn.normal, fsIn.fragPos));
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
    ambientLight.IrradianceMap = IrradianceMap;
    ambientLight.PrefilterMap = PrefilterMap;
    ambientLight.BRDF_Lut = BRDF_Lut;

    LightPair<LightArray<AreaLight, 4>, AmbientLight> AreaAndAmbient;
    AreaAndAmbient.first = _lightInfo.areaLights;
    AreaAndAmbient.second = ambientLight;

    lightingEnv.first = DirAndPoint;
    lightingEnv.second = AreaAndAmbient;

    //CookTorrence_GGX brdf;
    float3 V = normalize(fsIn.cameraPos - fsIn.fragPos);

    float3 finalColor;
    if(_shadowMapScreenSpace.Sample(fsIn.screenSpaceUv).x == 1.0){
        finalColor = lightingEnv.illuminate(g, brdf_, V);
    }else{
        finalColor = brdf_.albedo.xyz * 0.8;
    }

    return float4(finalColor, brdf_.albedo.w);

    //return float4(g.normal, 1.0);
}

