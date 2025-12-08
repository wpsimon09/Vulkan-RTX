import Core;
import Material;
import BxDF;
import Surface;
import Utils;
import RandomValues;

RWTexture2D _outputImage;

Sampler2D _positionBuffer;
Sampler2D _depthMap;
Sampler2D _normalMap;
Sampler2D _armMap;
Sampler2D _colourMap;

StructuredBuffer<ObjDescription> descriptions;
StructuredBuffer<Material> materialValues;
Sampler2D textures[];

ConstantBuffer<GlobalData> _globalData;

public SurfaceGeometry GetSurfaceInfo(uint32_t primitiveIndex, uint32_t instanceIndex, float2 baricentric, float3x4 objectToWorld)
{
    const float3 barycentricCoords = float3(1.0f - baricentric.x - baricentric.y, baricentric.x, baricentric.y);

    ObjDescription object = descriptions[instanceIndex];

    //=================================================
    // for each vertex there is one index that can
    // be used to look up the correct vertex in the array
    int i1 = object.indexAddress[primitiveIndex * 3];     // First index
    int i2 = object.indexAddress[primitiveIndex * 3 + 1]; // Second index
    int i3 = object.indexAddress[primitiveIndex * 3 + 2]; // Third index

    // Fetch the corresponding vertices
    Vertex v1 = object.vertexAddresses[i1];
    Vertex v2 = object.vertexAddresses[i2];
    Vertex v3 = object.vertexAddresses[i3];

    let features = materialValues[instanceIndex].Features;
    let values = materialValues[instanceIndex].Values;
    
    // Calculate normal using barycentric interpolation
    SurfaceGeometry g;
    float3 N = v1.norm * barycentricCoords.x + v2.norm * barycentricCoords.y + v3.norm * barycentricCoords.z;
    g.normal = N;

    float3x4 ObjToWorld = objectToWorld;
    
    g.shadingNormal = mul(ObjToWorld, float4(N, 0.0));
    
    float3 pos = v1.pos * barycentricCoords.x + v2.pos * barycentricCoords.y + v3.pos * barycentricCoords.z;
    g.position = pos;
    g.shadingPosition = mul(ObjToWorld, float4(pos, 1.0)).xyz;
    
    float2 uv = v1.uv.xy * barycentricCoords.x + v2.uv.xy * barycentricCoords.y + v3.uv.xy * barycentricCoords.z;
    g.uv = uv;
    
    float4 tangennt = float4(
        // interpolate tangent vector across the triangle 
        v1.tangent.xyz * barycentricCoords.x + v2.tangent.xyz * barycentricCoords.y + v3.tangent.xyz * barycentricCoords.z ,1.0);  // should we flip the orientation of tangent vector ? 
        
    float handedness = v1.tangent.w * barycentricCoords.x +
    v2.tangent.w * barycentricCoords.y +
    v3.tangent.w * barycentricCoords.z;
        
        
    tangennt.w = handedness;
    g.tangent = tangennt;

    

    g.TBN = CalculateTBN(g.normal, g.tangent, (float3x3)ObjToWorld);
    g.iTBN = transpose((g.TBN));

    if(features.hasNormalTexture){
        g.shadingNormal = normalize(SampleNormalMap(textures[features.normalTextureIdx],g.uv, g.TBN));
    }else{
        g.shadingNormal = normalize(mul(ObjToWorld, float4(N, 0.0)));
    }
    
    return g;
}


/*
Reflection map will be lerped in the composite pass based on its alpha channel
*/
[[numthreads(16,16,1)]]
[[shader("compute")]]
void computeMain(uint3 threadID : SV_DispatchThreadID){
    float2 dims;
    _outputImage.GetDimensions(dims.x, dims.y);
    float2 texCoords = (dims + 0.5) / threadID.xy;

    float depht = _depthMap.Sample(texCoords).x;

    if(depht == 1.0){
        _outputImage[threadID.xy] = float4(0.0);
        return;
    }

    float3 pos = _positionBuffer.Sample(texCoords).xyz;
    float3 arm = _armMap.Sample(texCoords).xyz;
    float3 albedo = _colourMap.Sample(texCoords).xyz;
    float3 N = _normalMap.Sample(texCoords).xyz;

    float3 wo = _globalData.cameraPostiion.xyz - pos;

    uint pixelID = threadID.x + threadID.y * (uint)dims.x;
    uint u = tea(pixelID, (uint)_globalData.currentFrame);
    
    PBRMaterial mat;
    mat.albedo = float4(albedo,1.0);
    mat.roughness = arm.x; 
    mat.metallness = arm.y;
    
    BxDFContext context;
    context.albedo = float4(albedo,1.0);
    mat.roughness = arm.x;
    mat.metallness = arm.y;

    SurfaceGeometry g;
    g.normal = N;
    g.shadingNormal = N;
    g.shadingPosition = pos;
    

    var sample = mat.Sample(context, wo, g, u);
    
    // we do not need to shoot ray from camear to world sice whe have the closest intersection in the postion buffer
    if(let sample = sample.brdf.Sample(wo, g, u)){
        RayDesc ray;
        ray.Origin = pos;    
        ray.Direction = sample.wi;
        ray.TMin = 0.0001;
        ray.TMax = 10000;

        //=========================
        // Trace ray and determine the reflected colour based on the surface hit 
        

    }



}