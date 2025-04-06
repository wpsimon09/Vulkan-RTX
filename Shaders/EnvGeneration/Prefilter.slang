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
        output.localPosition = input.position;

        return output;

    }

    [[shader("fragment")]]
    float4 fragmentMain(VertexOutput input) {

        return float4(1.0);
        
    }
