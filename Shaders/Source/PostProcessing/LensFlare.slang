import Core;
import Material;
import Utils;
import Light;

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> _globalData;

[[vk::binding(1, 0)]]
Sampler2D _inputTexture_BlueNoise;

[[vk::binding(2, 0)]]
ConstantBuffer<LightInfo> _light;

[[vk::binding(3, 0)]]
ConstantBuffer<PostProcessingParameters> _postProcessParameters;

[[vk::binding(0, 1)]]
Sampler2D _inputTexture_RenderedImage;

struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
    float2 screenSize : SCREEN_SIZE;
    nointerpolation float2 sunPosition : SUN_POS_VIEW_SPACE;
};

[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    // draw indexed with 3 vertices, requires no VAO binding
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    output.screenSize = _globalData.screenSize; 

    float4 sunPosWorld = _globalData.cameraPostiion - normalize(-_light.dirLight.direction) * 2000.0f;

    float4x4 viewProj = mul(_globalData.projection, _globalData.view);

    float4 sunClipSacpe = mul(viewProj, float4(sunPosWorld.xyz, 1.0));

    float3 sunNdc = sunClipSacpe.xyz / sunClipSacpe.w;

    bool sunVisible = sunClipSacpe.w > 0.0 && 
                  abs(sunNdc.x) <= 1.0 && 
                  abs(sunNdc.y) <= 1.0 && 
                  abs(sunNdc.z) <= 1.0;

    float2 sunUv = float2(sunNdc.x, -sunNdc.y) * 0.5 + 0.5;


    output.sunPosition = sunUv;

    //output.sunPosition = clamp(sunUv, 0.0, 1.0);

    return output;
}

float noise_float(float t)
{
	return _inputTexture_BlueNoise.Sample(float2(t,0.0)/_globalData.screenSize.xy).x;
}

float noise_float2(float2 t)
{
	return _inputTexture_BlueNoise.Sample(t/_globalData.screenSize.xy).x;
}

// lens flare from:
//https://godotshaders.com/shader/lens-flare-shader/
float3 LensFlare(float2 uv, float2 pos){
    float2 main = uv - pos;
    float2 uvd = uv * (length(uv));

    float ang = atan2(main.x, main.y);
    float dist = length(main);
    dist = pow(dist, 0.1);
    
    // Do not need an artificial sun
	float f0 = 1.0/(length(uv-pos)*16.0+1.0) ;
	f0 = f0 + f0*(sin(noise_float(sin(ang*2.+pos.x)*4.0 - cos(ang*3.+pos.y))*16.)*.1 + dist*.1 + .8);
    //f0 *= _light.dirLight.intensity.xyz;
	


    float n = noise_float2(float2(ang * 16.0, dist * 32.0));

    //=========================
    // each F represents different ghost with different screen space offset
    // ghosts are calculated fron uv coordinates and are projection on the screen 
    float f1 = max(0.01-pow(length(uv+1.2*pos),1.9),.0)*_postProcessParameters.f1Strength;

	float f2 = max(1.0/(1.0+32.0*pow(length(uvd+0.8*pos),2.0)),.0)*00.25;
	float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),.0)*00.23;
	float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9*pos),2.0)),.0)*00.21;
	
	float2 uvx = lerp(uv,uvd,-0.5);
	
	float f4 = max(0.01-pow(length(uvx+0.4*pos),2.4),.0) * _postProcessParameters.f2Strength;
	float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),.0)* (5.0 + _postProcessParameters.f2Strength);
	float f43 = max(0.01-pow(length(uvx+0.5*pos),2.4),.0)* (3.0 + _postProcessParameters.f2Strength);
	
	uvx = lerp(uv,uvd,-.4);
	
	float f5 = max(0.01-pow(length(uvx+0.2*pos),5.5),.0) * _postProcessParameters.f3Strength;
	float f52 = max(0.01-pow(length(uvx+0.4*pos),5.5),.0)*_postProcessParameters.f3Strength;
	float f53 = max(0.01-pow(length(uvx+0.6*pos),5.5),.0)*_postProcessParameters.f3Strength;
	
	uvx = lerp(uv,uvd,-0.5);
	
	float f6 = max(0.01-pow(length(uvx-0.3*pos),1.6),.0)*6.0;
	float f62 = max(0.01-pow(length(uvx-0.325*pos),1.6),.0)*3.0;
	float f63 = max(0.01-pow(length(uvx-0.35*pos),1.6),.0)*5.0;
	
	float3 c = float3(.0);
	
	c.r+=f2+f4+f5+f6; c.g+=f22+f42+f52+f62; c.b+=f23+f43+f53+f63;
	c = c*_postProcessParameters.lensFlareStrength - float3(length(uvd)*.05);
	
	// Do not need an artificial sun
	//c+=float3(f0) * (_light.dirLight.intensity.xyz * _light.dirLight.intensity.w) ;
	
	return c;
}

float3 cc(float3 color, float factor,float factor2) // color modifier
{
	float w = color.x+color.y+color.z;
	return lerp(color,float3(w)*factor,w*factor2);
}

[shader("fragment")]
float4 fragmentMain(VertexOut fsIn) : SV_Target {


    float2 uv = fsIn.uv;
    float2 resolution = 1.0 / _globalData.screenSize;

    float4 previousColour = _inputTexture_RenderedImage.Sample(uv);
    float3 col = previousColour.xyz;

    float2 sunDist = fsIn.sunPosition - 0.5;
    float sunFalloff = 1.0 - smoothstep(0.9, 1.5, length(sunDist)); // fade out after screen edges


    col += float3(1.4,1.2,1.0) * LensFlare(uv, fsIn.sunPosition) ;
    col -= noise_float2(fsIn.pos.xy) * 0.015;
    col = cc(col, 0.5, 0.1);
    
    return float4( col, 1.0);
}

