struct FragmentShaderIn
{
    float2 uv : OUT_UV;
};

[[vk::binding(3, 0)]]
ConstantBuffer<Sampler2D> _albedoMap;

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn)
    : SV_Target
{

    float4 bilboardTexture = _albedoMap.Sample(fsIn.uv);
    if (bilboardTexture.w < 1.0) {
        discard;
    }

    return float4(bilboardTexture.xyz, 1.0);
}

