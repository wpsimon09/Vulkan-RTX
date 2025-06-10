module SBG_ForwardLitNoMaterials;

import Light;
import Core;

//=================================
// Set 0

[[vk::binding(0,0)]]
ConstantBuffer<GlobalData> globalData;

[[vk::binding(1, 0)]]
StructuredBuffer<PermodelData> perModelData;