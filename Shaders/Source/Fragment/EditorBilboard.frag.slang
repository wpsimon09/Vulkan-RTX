import Material;
#include "ShaderBindingGroups/ForwardUnlit.slang"

struct FragmentShaderIn
{
    float2 uv : OUT_UV;
    Material mat : OUT_MAT;

};

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn)
    : SV_Target
{

    float4 bilboardTexture = textures[fsIn.mat.Features.albedoTextureIdx].Sample(fsIn.uv);
    if (bilboardTexture.w < 1.0) {
        discard;
    }

    return float4(bilboardTexture.xyz, 1.0);
}

