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
    float2 screenSpaceUV: OUT_SCREEN_UV;
};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;
    float4 sv_position : SV_Position;
}

[shader("pixel")]
float4 fragmentMain(VertexOutput fsIn)
    : SV_Target
{
   SurfaceGeometry g;
    g.position = fsIn.vertexData.fragPos;
    g.shadingPosition = fsIn.vertexData.fragPos;
    g.shadingNormal = normalize(fsIn.vertexData.normal);
    g.normal = normalize(fsIn.vertexData.normal);
    g.uv = fsIn.vertexData.uv;

    Material mat = materialValues[fsIn.vertexData.materialIdx];

    PBRMaterial material;
    material.features = mat.Features;
    material.values = mat.Values;
    material.albedo_map = textures[mat.Features.albedoTextureIdx];
    material.arm_map = textures[mat.Features.armTextureIdx];
    material.normal_map = textures[mat.Features.normalTextureIdx];
    material.emissive_map = textures[mat.Features.emissiveTextureIdx];
    let brdf_ = material.Prepare(g);

    if (material.features.hasNormalTexture) {
        g.normal = normalize(SampleNormalMap(material.normal_map, fsIn.vertexData.uv, fsIn.vertexData.normal, fsIn.vertexData.fragPos));
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
    float3 V = normalize(fsIn.vertexData.cameraPos - fsIn.vertexData.fragPos);

    float3 finalColor;
    float shadowFactor = _shadowMapScreenSpace.Sample(fsIn.sv_position.xy / fsIn.vertexData.screenSpaceUV ).x;
    if( shadowFactor > 0.1){
        finalColor = lightingEnv.illuminate(g, brdf_, V) * shadowFactor;
    }else{
        finalColor = ambientLight.illuminate(g, brdf_,V ) * 0.5;
    }

    return float4(finalColor, 1);
}

