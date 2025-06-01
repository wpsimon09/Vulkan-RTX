module SBG_ForwardUnlit;
import Core;
import Material;

//========================
// set 0

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> globalData;

[[vk::binding(1, 0)]]
StructuredBuffer<PermodelData> perModelData ;

[[vk::binding(2, 0)]]
StructuredBuffer<Material> materials;

//========================
// set 1

[[vk::binding(0, 1)]]
Sampler2D textures[];
