struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
    float2 uv : OUT_UV;
};

struct TextureSampler{
    Texture2D texture;
    SamplerState sampler;
    float4 Sample(float2 uv) {
        return texture.Sample(sampler, uv);
    }
}

[[vk::binding(2,0)]]
ConstantBuffer<TextureSampler> albedo;

[shader("pixel")]
float4 fragmentMain(FragmentShaderIn fsIn) : SV_Target {
    float4 fragColor;
    fragColor = albedo.Sample(fsIn.uv);

    return fragColor; 
}

