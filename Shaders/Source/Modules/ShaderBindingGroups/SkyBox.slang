module SBG_SkyBox;
import Core;

//=================================
// Set 0

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> globalData;

//==================================
// Set 1
[[vk::binding(0, 1)]]
ConstantBuffer<SamplerCube> _skyBoxHDR;
