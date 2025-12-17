module SBG_SkyBox;
import Core;

//=================================
// Set 0

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData2> globalData;

//==================================
// Set 1
[[vk::binding(0, 1)]]
ConstantBuffer<SamplerCube> _skyBoxHDR;
