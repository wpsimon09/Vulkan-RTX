/**
Renders the acctual athosphere 
based on https://github.com/MatejSakmary/atmosphere-bac/blob/rewrite/shaders/multiscatteringLUT.glsl
**/
import Atmosphere;
import AtmosphereParameters;
import Utils;
import Core;
import Light;
import Intersections;

static const float cameraScale = 0.1;

struct VertexOut {
    float4 pos : SV_Position;
    float2 uv : TEXCOORDS;
    float2 screenSize : SCREEN_SIZE;
};

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> globalData;

[[vk::binding(1, 0)]]
ConstantBuffer<LightInfo> light;

[[vk::binding(2, 0)]]
Sampler2D depthTexture;

[[vk::binding(3, 0)]]
Sampler2D skyViewLut;

[[vk::binding(4, 0)]]
Sampler2D transmitanceLut;

[[vk::push_constant]]
AtmosphereParamsCPU atmosphereParams;

float3 sunWithHalo(float3 worldDir, float3 sunDir){
    
    const float sunSolidAngle = 1.0 * PI / 180.0;
    const float minSunCosTheta = cos(sunSolidAngle);

    float cosTheta = dot(worldDir, -sunDir);
    if(cosTheta >= minSunCosTheta) {return float3(0.5) ;}
    float offset = minSunCosTheta - cosTheta;
    float gaussianBloom = exp(-offset * 50000.0) * 0.5;
    float invBloom = 1.0/(0.02 + offset * 300.0) * 0.01;
    return float3(gaussianBloom + invBloom);
}

[shader("vertex")]
VertexOut vertexMain(uint VertexIndex: SV_VertexID) {
    // draw indexed with 3 vertices, requires no VAO binding
    VertexOut output;
    output.uv = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    output.screenSize = globalData.screenSize; 
    return output;
}

[[shader("fragment")]]
float4 fragmentMain(VertexOut fsIn){

    AtmosphereParamsGPU atmosphere = atmosphereParams.Parse();

    const float3 camera = globalData.cameraPostiion.xyz;
    const float3 sunDir = -light.dirLight.direction.xyz;
    float2 skyViewDims;
    skyViewLut.GetDimensions(skyViewDims.x, skyViewDims.y);

    float2 atmosphereBoundareis = float2(atmosphere.radiusBottom, atmosphere.radiusTop);

    // tansform pixel to the world pos 
    float4x4 invViewProjMat = mul(globalData.inverseView , globalData.invProjection);
    float2 pixPos = fsIn.uv;
    float3 clipSpace = float3(pixPos * float2(2.0) - float2(1.0), 1.0);
    float4 Hpos = mul(invViewProjMat, float4(clipSpace, 1.0));

    float3 worldDir = normalize(Hpos.xyz / Hpos.w - camera);
    float3 worldPos = camera * cameraScale + float3(0, atmosphere.radiusBottom,0.0);

    float viewHeight = length(worldPos);
    float3 L = float3(0.0);

    // mask the depth 
    float depth = depthTexture.Sample(fsIn.uv).x;

    if(depth == 1.0){
        float2 uv; // uvs for sampling the sky view lut 
        float3 upVector = normalize(worldPos);
        // zenith is needed to fetch the colour of the sky 
        float viewZenithAngle = acos(dot(worldDir, upVector));

        float lightViewAngle = acos(dot(normalize(float3(sunDir.xz, 0.0)), normalize(float3(worldDir.xz, 0.0))));

        bool intersectsGround = raySphereIntersectNearest(worldPos, worldDir, float3(0.0), atmosphere.radiusBottom) >= 0;

        uv = SkyViewLutParamsToUv(intersectsGround, float2(viewZenithAngle, lightViewAngle),viewHeight, atmosphereBoundareis, skyViewDims );

        L += skyViewLut.Sample(uv).xyz;

        // if des not ray intersect the ground and thus goes all the way to the atmosphere, render the small sun 
        if(!intersectsGround){
            if(atmosphere.DrawSun){
                // the lut params for Transmitance texture look up,
                // they are altitude and zenith angle, zenit angle between vertical vector and sun direction
                float2 transLutParams = float2(length(worldPos), dot(-sunDir, upVector));
                
                // get coordinates to sample from transmitance texture calculated earliear 
                float2 transmitanceUV = GetTransmittanceTextureUvFromRMu(atmosphere, transLutParams.x, transLutParams.y);
                
                // if we didnt have it precompute we would have to ray march towards sun as well 
                float3 transmitanceToSun = transmitanceLut.Sample(transmitanceUV).xyz;

      
                L += (sunWithHalo(worldDir, sunDir) * transmitanceToSun) ;
            }
        }

        return float4(L * light.dirLight.intensity.w, 1.0);
    }else{
        return float4(0.2, 0.3, 0.2, 0.0);
    }
}