import Core;
import Material;
import Utils;
#include "ShaderBindingGroups/PostProcessing.slang"

struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
};

[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    // draw indexed with 3 vertices, requires no VAO binding
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) :SV_Target {
    
    float3 hdrColor = _inputTexture_Render.Sample(fsIn.uv).xyz;
    
    
    if(_inputTexture_Depth.Sample(fsIn.uv).r == 1.0 && !_toneMappigParams.isRayTracing){
        float3 mapped = aces_film(hdrColor); //float3(1.0) - exp(-hdrColor * postProcessingParameters.toneMapping.x);
        return float4(mapped, 1.0);
    }
    float3 mapped = aces_film(hdrColor * _toneMappigParams.exposure); //float3(1.0) - exp(-hdrColor * postProcessingParameters.toneMapping.x);
    return float4(mapped, 1.0);
}
