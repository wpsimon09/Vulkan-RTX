struct FragmentShaderIn {
    float3 localPos : OUT_LOCAL_POS;
    float2 uv : OUT_UV;
};

[[vk::binding(0, 3)]]
ConstantBuffer<Sampler2D> _skyBoxHDR;

const float2 invAtan = float2(0.1591, 0.3183);

float2 SampleSphericalMap(float3 dir, float3 localPos) {
    float2 uv = float2(atan2(dir.x, dir.y), asin(dir.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}


[shader("fragment")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {
    float2 uv = SampleSphericalMap(normalize(fsIn.localPos), fsIn.localPos);
    float3 skyboxColour = _skyBoxHDR.Sample(uv).xyz;
    return float4(skyboxColour, 1.0);
}