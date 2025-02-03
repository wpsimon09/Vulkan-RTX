import Material;

struct FragmentShaderIn
{
    float2 uv : OUT_UV;
};

[[vk::binding(4, 0)]]
ConstantBuffer<Sampler2D> _albedoMap;

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn)
    : SV_Target
{
    SurfaceGeometry g;
    g.position = fsIn.fragPos;
    g.normal = normalize(fsIn.normal);
    g.uv = fsIn.uv;

    float4 bilboardTexture = _albedoMap.Sample(g.uv);
    if (bilboardTexture.w < 1.0) {
        discard;
    }

    return float4(bilboardTexture.xyz, 1.0);
}

