module SBG_PostProcessing;

import Core;

[[vk::binding(0, 0)]]
ConstantBuffer<PostProcessingParameters> postProcessingParameters;

[[vk::binding(0, 1)]]
Sampler2D _inputTexture_Render;


