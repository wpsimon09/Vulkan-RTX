    struct HDRToCubeMapPushData
    {
        float4x4 projectionView;
    };

    [[vk::binding(0, 0)]] ConstantBuffer<HDRToCubeMapPushData> pushData;

    [[vk::binding(3, 0)]] ConstantBuffer<Sampler2D> hdrMap;

    struct VertexInput 
    {
        float3 position : POSITION;
    };

    struct VertexOutput
    {
        float3 localPosition : LOCAL_POS;
        float4 position : SV_Position;
    };
    
    static const float2 invAtan = float2(0.1591, 0.3183);
    float2 SampleSphericalMap(float3 v)
    {
        float2 uv = float2(atan2(-v.z, v.x), asin(v.y));
        uv *= invAtan;
        uv += 0.5;
        return float2(uv.x, 1.0 - uv.y);
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
        float2 skyBoxUV = SampleSphericalMap(normalize(input.localPosition));
        float3 skyboxColour = hdrMap.Sample(skyBoxUV).xyz;

        float3 mapped = skyboxColour / (skyboxColour + float3(1.0));
        return float4(mapped, 1.0);        
    }
