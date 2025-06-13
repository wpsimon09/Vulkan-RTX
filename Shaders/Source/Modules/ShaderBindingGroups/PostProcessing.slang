module SBG_PostProcessing;

import Core;

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> globalData;

[[vk::binding(1, 0)]]
Sampler2D _inputTexture_Render;


