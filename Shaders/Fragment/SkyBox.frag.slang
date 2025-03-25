struct FragmentShaderIn {
    float3 localPos : OUT_LOCAL_POS;
    float2 uv : OUT_UV;
};

[shader("fragment")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {
    return float4(1.0, 0.0, 0.0, 1.0);
}