struct VertexInput {
    float3 inPosition :POSITION ;
    float3 inNormal : NORMAL;
    float2 inUV : UV;
};

struct FragmentShaderIn {
    float3 normal : OUT_NORMAL;
    float3 cameraPos : OUT_CAM_POS;
    float3 fragPos : OUT_FRAG_POS;
    float3 lightPos : OUT_LIGHT_POS;
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

    float4 lightPosition;
    float4 cameraPostiion;
    float4 viewParams;

    float padding;
    float padding2;
    float2 screenSize;
};

[[vk::binding(0,0)]]
ConstantBuffer<GlobalData> globalData;

struct PermodelData {
    column_major float4x4 modelMatrix;
    column_major float4x4 normalMatrix;
};

[[vk::binding(1,0)]]
ConstantBuffer<PermodelData> perModelData ;

[shader("vertex")]
VertexOutput vertexMain(VertexInput VAO) 
{
    // Input data
    float3 vertexPos = VAO.inPosition; 
    float3 normal = VAO.inNormal;     
    float2 uv = VAO.inUV;             

    // Compute world position of the quad's center
    float3 quadCenterWorldPos = mul(perModelData.modelMatrix, float4(0.0, 0.0, 0.0, 1.0)).xyz;

    // Compute camera direction (camera position - quad center position)
    float3 cameraToQuad = globalData.cameraPostiion.xyz - quadCenterWorldPos;
    cameraToQuad = normalize(cameraToQuad);

    // Construct billboard-facing coordinate system
    float3 up = float3(0.0, 1.0, 0.0); // World up vector
    float3 right = normalize(cross(up, cameraToQuad)); // Right vector
    float3 billboardUp = normalize(cross(cameraToQuad, right)); // Recompute up vector

    // Adjust vertex position to face the camera (billboard effect)
    float3 billboardVertexPos = quadCenterWorldPos + ((vertexPos.x * right * 4)+ (vertexPos.y * billboardUp *4));
    

    // Set up output
    VertexOutput out;
    out.vertexData.cameraPos = globalData.cameraPostiion.xyz;
    out.vertexData.fragPos = billboardVertexPos; 
    out.vertexData.lightPos = globalData.lightPosition.xyz;
    out.vertexData.normal = normal; 
    out.vertexData.uv = uv;

    // Compute the final position on screen using the view-projection matrix
    float4x4 viewProj = mul(globalData.view, globalData.projection);
    out.sv_position = mul(viewProj, float4(billboardVertexPos, 1.0));

    return out;
}