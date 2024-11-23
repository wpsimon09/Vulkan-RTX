struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
};

[[vk::binding(2,0)]]
ConstantBuffer<Texture2D> albedo;

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn ) :SV_Target {
    return float4(1.0, 1.0,0.0,1.0); 
}

