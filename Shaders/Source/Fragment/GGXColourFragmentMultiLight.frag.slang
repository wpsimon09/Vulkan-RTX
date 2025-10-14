import Material;
import Light;
import Surface;
import Atmosphere;
#include "ShaderBindingGroups/ForwardLit.slang"

struct FragmentShaderIn
{
    float3 normal : OUT_NORMAL; 
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float2 uv : OUT_UV;
    int materialIdx : OUT_MATERIAL;
    float2 screenSpaceUV: OUT_SCREEN_UV;
    float3x3 TBN : OUT_TBN;

};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;
    float4 sv_position : SV_Position;
}

static const float cameraScale = 0.1;
// TODO: figure out how to pass atmosphere botom radius

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
    g.TBN = fsIn.vertexData.TBN;
    g.iTBN = transpose(fsIn.vertexData.TBN);

    Material mat = materialValues[fsIn.vertexData.materialIdx];

    PBRMaterial material;
    material.features = mat.Features;
    material.values = mat.Values;
    material.albedo_map = textures[mat.Features.albedoTextureIdx];
    material.arm_map = textures[mat.Features.armTextureIdx];
    material.normal_map = textures[mat.Features.normalTextureIdx];
    material.emissive_map = textures[mat.Features.emissiveTextureIdx];

    let brdf_ = material.Prepare(g);

    if(material.features.hasNormalTexture){
        g.shadingNormal = SampleNormalMap(material.normal_map, g.uv, fsIn.vertexData.TBN);
    }

    LightPair<LightPair<DirectionalLight, LightArray<PointLight, 20>>, LightPair<LightArray<AreaLight,4>,AmbientLight>> lightingEnv;
    LightPair<DirectionalLight, LightArray<PointLight, 20>> DirAndPoint;
    DirAndPoint.first = _lightInfo.dirLight;
    DirAndPoint.second = _lightInfo.pointLights;


    LightPair<LightArray<AreaLight,4>,AmbientLight> AreaAndAmbient;

    AmbientLight ambientLight;
    ambientLight.albedo = brdf_.albedo.xyz;
    ambientLight.aoIntensity = _lightInfo.info.y;
    ambientLight.roughness = brdf_.roughness;
    ambientLight.applyIBL = bool(_lightInfo.info.x);
    ambientLight.IrradianceMap = IrradianceMap;
    ambientLight.PrefilterMap = PrefilterMap;
    ambientLight.BRDF_Lut = BRDF_Lut;

    AreaAndAmbient.first = _lightInfo.areaLights;
    AreaAndAmbient.second = ambientLight;

    lightingEnv.first = DirAndPoint;
    lightingEnv.second = AreaAndAmbient;

    //CookTorrence_GGX brdf;
    float3 V = normalize(fsIn.vertexData.cameraPos - fsIn.vertexData.fragPos);

    float3 finalColor;
    float2 shadowFactor = _shadowMapScreenSpace.Sample(fsIn.sv_position.xy / fsIn.vertexData.screenSpaceUV ).xy;

    
    float3 transmitanceToSun = float3(1.0);
    if(globalData.accountForSunTransmitance){
        float3 worldPosInAtmosphere = fsIn.vertexData.fragPos * cameraScale + float3(0.0, globalData.atmosphereInfo.x , 0.0);
        float height = length(worldPosInAtmosphere);
        float3 upVector = worldPosInAtmosphere / height;
        // TODO: do i reall need to revert the light direction ? 
        float viewZenithAngle = dot(-lightingEnv.first.first.direction.xyz, upVector);
        float2 transLutParams = float2(height, viewZenithAngle);
        
        float2 transmitanceTextureWH;
        TransmitanceLUT.GetDimensions(transmitanceTextureWH.x, transmitanceTextureWH.y);
        
        float2 trnansUv = GetTransmittanceTextureUvFromRMu(float2(globalData.atmosphereInfo.x,globalData.atmosphereInfo.y), transmitanceTextureWH, height, viewZenithAngle);
        transmitanceToSun = TransmitanceLUT.Sample(trnansUv).xyz;
        
        lightingEnv.first.first.transmitanceToSun = transmitanceToSun;
    }
    
    finalColor = lerp(ambientLight.illuminate(g, brdf_, V) * 0.2, lightingEnv.illuminate(g, brdf_, V)  + ambientLight.illuminate(g, brdf_, V),   shadowFactor.x);
    finalColor.xyz += (brdf_.emission.xyz * brdf_.emission.w);

    // shadow factor.y is an ambient occlusion data 
    finalColor.xyz *= shadowFactor.y;

    switch(globalData.renderOutput){
        case ERenderOutput::FullLit :{
            return float4(finalColor, 1);
        }
        case ERenderOutput::NormalVectors:{
            return float4(g.shadingNormal, 1.0);
        }
        case ERenderOutput::LightOnly:{
            return float4(finalColor,1.0);
        };
        case ERenderOutput::Albedo:{
            return float4(brdf_.albedo.xyz, 1.0);
        }
        case ERenderOutput::Arm:{
            return float4(brdf_.ao, brdf_.roughness, brdf_.metallnes, 1.0);
        }
        default: {
            return float4(1.0);
        }
    }
    
}

