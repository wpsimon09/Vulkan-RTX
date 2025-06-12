module SBG_ShadowPass;
import Core;
import Light;

//=================================
// Set 0
[[vk::binding(0,0)]]
ConstantBuffer<GlobalData> globalData;

// those two are for shadow calculations 
[[vk::binding(1, 0)]]
ConstantBuffer<LightInfo> _lightInfo;

[[vk::binding(2, 0)]]
RaytracingAccelerationStructure _tlas;

[[vk::binding(3, 0)]]
Sampler2D _depthTexture;
