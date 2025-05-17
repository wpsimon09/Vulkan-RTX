struct VertexInput {
    float3 inPosition :POSITION ;
};

struct FragmentShaderIn {
    float3 localPos : OUT_LOCAL_POS;
    float2 uv : OUT_UV;
};

struct VertexOutput {
    FragmentShaderIn vertexData : DATA;
    float4 sv_position : SV_Position;
}

struct GlobalData {
    column_major float4x4 projection;
    column_major float4x4 view;
    float4x4 inverseView;
    float4x4 invProjection;

    float4 lightPosition;
    float4 cameraPostiion;
    float4 viewParams;

    float padding;
    float padding2;
    float2 screenSize;
};

[[vk::binding(0,0)]]
ConstantBuffer<GlobalData> globalData;


//=================================
// MATERIAL STRUCTS
//=================================
struct MaterialConstantValues
{
    float roughness;
    float metalness;
    float ao;
    float padding;

    float4 diffuse;

    float4 emissive_strength;
}

struct MaterialFeatures
{
    bool hasDiffuseTexture;
    bool hasEmissiveTexture;
    bool hasNormalTexture;
    bool hasArmTexture;
}

struct Material
{
    MaterialConstantValues Values;
    MaterialFeatures Features;
};

struct PermodelData {
    column_major float4x4 modelMatrix;
    column_major float4x4 normalMatrix;
    float3 positionWorldSpace;
    float padding;

    Material material;
};

[[vk::binding(1,0)]]
ConstantBuffer<PermodelData> permodelData;


[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    // sotre for later
    float3 vertexPos = VAO.inPosition;

    VertexOutput out;
    out.vertexData.localPos = vertexPos;
    
    column_major float4x4 rotView;
    rotView[0] = float4(globalData.projection[0].xyz, 0.0);
    rotView[1] = float4(globalData.projection[1].xyz, 0.0);
    rotView[2] = float4(globalData.projection[2].xyz, 0.0);
    rotView[3] = float4(0.0, 0.0, 0.0, 1.0);
    
    
    //float4x4 proj = globalData.projection;

    column_major float4x4 viewProj = mul(globalData.view, rotView);
    float4 pos = mul(viewProj, float4(VAO.inPosition, 1.0));
    out.sv_position = pos.xyww;


    return out;
}   
