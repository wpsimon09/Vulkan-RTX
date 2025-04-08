    struct HDRToCubeMapPushData
    {
        float4x4 projectionView;
        float4 params; // x - roughness | yzw - padding
    };
    
    static const float PI = 3.14159265359;

    [[vk::binding(0, 0)]] ConstantBuffer<HDRToCubeMapPushData> pushData;

    [[vk::binding(3, 0)]] ConstantBuffer<SamplerCube> hdrCubeMap;

    struct VertexInput 
    {
        float3 position : POSITION;
    };

    float RadicalInverse_VdC(uint bits)
    {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return float(bits) * 2.3283064365386963e-10; // / 0x100000000
    }
    // ----------------------------------------------------------------------------
    float2 Hammersley(uint i, uint N)
    {
        return float2(float(i) / float(N), RadicalInverse_VdC(i));
    }

    float3 importanceSampleGGX(float2 Xi, float3 N, float roughness)
    {
        float a = roughness * roughness;

        // create spherical coordinates for sample direction
        float phi = 2.0 * PI * Xi.x;
        float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        // convert to the cartesian
        float3 H;
        H.x = cos(phi) * sinTheta;
        H.y = sin(phi) * sinTheta;
        H.z = cosTheta;

        // convertes from cartesian tangent space to world space
        float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
        float3 tangent = normalize(cross(up, N));
        float3 bitangent = cross(N, tangent);

        float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
        return normalize(sampleVec);
    }

    struct VertexOutput
    {
        float3 localPosition : LOCAL_POS;
        float roughness : ROUGHNESS;
        float4 position : SV_Position;
    };


    [[shader("vertex")]]
    VertexOutput vertexMain(VertexInput input) {
        VertexOutput output;
        output.position = mul(pushData.projectionView, float4(input.position, 1.0));
        output.roughness = pushData.params.x;
        output.localPosition = input.position;

        return output;

    }

    float D_GGX(float dotNH, float roughness)
    {
        float alpha = roughness * roughness;
        float alpha2 = alpha * alpha;
        float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
        return (alpha2) / (PI * denom * denom);
    }

    [[shader("fragment")]]
    float4 fragmentMain(VertexOutput input) {

        float3 N = normalize(input.localPosition).xyz;
        float3 R = N;
        float3 V = R;

        const uint SAMPLE_COUNT = 1024u;
        float totalWeight = 0.0f;
        float3 prefilteredColor = float3(0.0);

        for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
            // generate random sample
            float2 Xi = Hammersley(i, SAMPLE_COUNT);
            float3 H = importanceSampleGGX(Xi, N, input.roughness);
            float3 L = normalize(2.0 * dot(V, H) * H - V);

            float NdotL = max(dot(N, L), 0.0);

            if (NdotL > 0.0) {
                float dotNH = clamp(dot(N, H), 0.0, 1.0);
                float dotVH = clamp(dot(V, H), 0.0, 1.0);

                // float pdf = D_GGX(dotNH, input.roughness) * dotNH / (4.0 * dotVH) + 0.0001;

                // float resolution = 512.0f;
                // float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
                // float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

                // float mipLevel = 0;

                float D = D_GGX(NdotL, input.roughness);
                float pdf = (D * dotNH / (4 * dotVH)) + 0.0001;

                float resolution = 1024;
                float saTesel = 4.0 * PI / (6.0 * resolution * resolution);
                float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001); 

                float mipLevel = input.roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTesel);

                prefilteredColor += hdrCubeMap.SampleLevel(L, mipLevel).xyz * NdotL;
                totalWeight += NdotL;
            }

        }
        prefilteredColor = prefilteredColor  / totalWeight;

        return float4(prefilteredColor, 1.0);
        //return float4(hdrCubeMap.Sample(input.localPosition).xyz, 1.0);
    
    }
