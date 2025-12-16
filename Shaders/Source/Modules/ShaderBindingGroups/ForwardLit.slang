module SBG_ForwardLigt;
import Core;
import Material;
import Light;

//========================
// set 0

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData2> globalData;

[[vk::binding(1,0)]]
StructuredBuffer<PermodelData, Std430DataLayout> perObjectData ;

[[vk::binding(2, 0)]]
StructuredBuffer<Material> materialValues;

[[vk::binding(3, 0)]]
ConstantBuffer<LightInfo> _lightInfo;

[[vk::binding(4,0)]]
Sampler2D LTC;

[[vk::binding(5,0)]]
Sampler2D LTC_Inverse;

[[vk::binding(6,0)]]
Sampler2D TransmitanceLUT;

[[vk::binding(7, 0)]]
Sampler2D _directionalLightShadowMap;

[[vk::binding(8, 0)]]
Sampler2D _abmbientOcclusionMap;


//========================
// set 1

[[vk::binding(1, 1)]]
Sampler2D textures[];

[[vk::binding(2, 1)]]
ConstantBuffer<SamplerCube> IrradianceMap;

[[vk::binding(3, 1)]]
ConstantBuffer<SamplerCube> PrefilterMap;

[[vk::binding(4, 1)]]
ConstantBuffer<Sampler2D> BRDF_Lut;


