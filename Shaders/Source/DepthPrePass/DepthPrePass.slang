struct VertexInput {
    float3 inPosition :POSITION ;
};

struct VertexOutput {
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

    float useEnvLighting;
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

struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float2 uv : OUT_UV;
    Material material : OUT_MATERIAL;
};

[[vk::binding(1,0)]]
ConstantBuffer<PermodelData> perModelData ; 

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) // VAO input{
{
    // sotre for later
    float3 vertexPos = VAO.inPosition;

    float3 FragmentWorldPos = mul(perModelData.modelMatrix, float4(vertexPos, 1.0)).xyz;

    VertexOutput out;
    
    float4x4 viewProj = mul(globalData.view, globalData.projection);
    out.sv_position = mul(viewProj, float4(FragmentWorldPos, 1.0));

    return out;
}

[shader("fragment")]
void fragmentMain() {

}
