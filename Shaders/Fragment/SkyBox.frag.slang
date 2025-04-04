struct FragmentShaderIn {
    float3 localPos : OUT_LOCAL_POS;
};

[[vk::binding(3, 0)]]
ConstantBuffer<SamplerCube> _skyBoxHDR;

const float2 invAtan = float2(0.1591, 0.3183);

float2 SampleSphericalMap(float3 dir) {
    float2 uv = float2(atan2(-dir.z, dir.x), asin(dir.y));
    uv *= invAtan;
    uv += 0.5;
    return float2(uv.x, 1.0 - uv.y);
}


[shader("fragment")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {
    float2 skyBoxUV = SampleSphericalMap(normalize(fsIn.localPos));


//    float3 skyboxColour = _skyBoxHDR.Sample(skyBoxUV).xyz;

    float3 skyboxColour = _skyBoxHDR.Sample(fsIn.localPos).xyz;

    float3 mapped = skyboxColour / (skyboxColour + float3(1.0));
    return float4(mapped, 1.0);
}