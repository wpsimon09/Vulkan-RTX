module SBG_ForwardLitNoMaterials;

import Light;
import Core;
import Material;



    
//=================================
// Set 0
[[vk::binding(0,0)]]
ConstantBuffer<GlobalData2> globalData;

//===================================
// Set 1 

[[vk::binding(0, 1)]]
StructuredBuffer<Material> materialValues;

[[vk::binding(1, 1)]]
Sampler2D textures[];

[[vk::binding(2, 1)]]
StructuredBuffer<PermodelData, Std430DataLayout> perObjectData ;

