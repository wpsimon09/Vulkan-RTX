import Core;
import Material;
import Utils;
import ToneMappingOperations;
import ColourSpaceConversions;

[[vk::push_constant]]
ToneMappingParameters _toneMappigParams;

[[vk::binding(0, 1)]]
Sampler2D _inputTexture_Render;

[[vk::binding(1, 1)]]
Texture2D _inputeTexture_AverageLuminance;

[[vk::binding(0, 0)]]
Sampler2D _inputTexture_Depth; // used to determine if we are tone mapping background

enum EToneMappingCurve
{
    Aces_Approc = 0,
    Uchimura,
    Aces_Full
};


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
    if(_inputTexture_Depth.Sample(fsIn.uv).x == 1.0 && !(bool)_toneMappigParams.isRayTracing){
        return float4(tone_map_aces_approx(hdrColor),1.0);
    }

    float avgLuminance = _inputeTexture_AverageLuminance.Load(uint3(0,0,0)).x;

    float3 Yxy = convertRGB2Yxy(hdrColor); // Yxy - the Y is luminance
    float whitePoint = _toneMappigParams.whitePoint;

    // exposure 
    Yxy.x /= (9.6 * avgLuminance + 0.00001);

    let rgb = convertYxy2RGB(Yxy);
    
    float3 toneMapped;
    
    switch((EToneMappingCurve)_toneMappigParams.curve){
        case EToneMappingCurve::Aces_Approc:{
            toneMapped.x = aces_film(rgb.x);
            toneMapped.y = aces_film(rgb.y);
            toneMapped.z = aces_film(rgb.z);
            break;
        }
        case EToneMappingCurve::Uchimura:{
            toneMapped.x = Tonemap_Uchimura(rgb.x);
            toneMapped.y = Tonemap_Uchimura(rgb.y);
            toneMapped.z = Tonemap_Uchimura(rgb.z);
            break;
        }
        case EToneMappingCurve::Aces_Full:{
            toneMapped = Aces_Full(rgb);
            break;
        }
    }
    
    
    return float4(toneMapped, 1.0);

}
