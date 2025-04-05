    struct HDRToCubeMapPushData
    {
        float4x4 projectionView;
    };
    
    static const float PI = 3.14159265359;

    [[vk::binding(0, 0)]] ConstantBuffer<HDRToCubeMapPushData> pushData;

    [[vk::binding(3, 0)]] ConstantBuffer<SamplerCube> hdrCubeMap;

    struct VertexInput 
    {
        float3 position : POSITION;
    };

    struct VertexOutput
    {
        float3 localPosition : LOCAL_POS;
        float4 position : SV_Position;
    };

    float radicalInverse_VdC(uint bits)
    {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return float(bits) * 2.3283064365386963e-10; // / 0x100000000
    }

    [[shader("vertex")]]
    VertexOutput vertexMain(VertexInput input) {
        VertexOutput output;
        output.position = mul(pushData.projectionView, float4(input.position, 1.0));
        output.localPosition = input.position;

        return output;

    }

    [[shader("fragment")]]
    float4 fragmentMain(VertexOutput input) {
        float3 normal = normalize(input.localPosition);
        float3 irradiance = float3(0.0f);

        float3 up = float3(0.0, 1.0, 0.0);
        float3 right = cross(up, normal);
        up = cross(normal, right);

        float sampleDelta = 0.005; // how much each iterration moves along the sphere 
        float nrSamples = 0.0;

        float deltaPhi = (2.0f * float(PI)) / 180.0f;
        float deltaTheta = (0.5f * float(PI)) / 64.0f;

        for (float phi = 0.0f; phi < 2.0 * PI; phi += deltaPhi) { // mowe down along sphere
            for (float theta = 0.0; theta < 0.5 * PI; theta += deltaTheta) { // move right along spher e

                // convertes from spherical to cartesian syste
                float3 tempVec = cos(phi) * right + sin(phi) * up;
                float3 sampleVector = cos(theta) * normal + sin(theta) * tempVec;
                irradiance += hdrCubeMap.Sample(sampleVector).rgb * cos(theta) * sin(theta);
                
                nrSamples++;
                                
            }
        }

        irradiance = PI * irradiance * (1.0 / float(nrSamples));

        return float4(irradiance, 1.0);
        
    }
