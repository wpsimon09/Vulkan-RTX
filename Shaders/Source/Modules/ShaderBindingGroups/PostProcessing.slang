module SBG_PostProcessing;

import Core;

[[vk::push_constant]]
ToneMappingParameters _toneMappigParams;

[[vk::binding(0, 1)]]
Sampler2D _inputTexture_Render;

[[vk::binding(1, 1)]]
Texture2D _inputeTexture_AverageLuminance;


[[vk::binding(0,0)]]
Sampler2D _inputTexture_Depth; // used to determine if we are tone mapping background


