import Core;
import Material;
import BxDF;
import Surface;
import Utils;
import RandomValues;
import RayTracingUtils;

//========================
// set 0 

[[vk::binding(0,0)]]
RWTexture2D _outputImage;

[[vk::binding(1,0)]]
Sampler2D _positionBuffer;

[[vk::binding(2,0)]]
Sampler2D _depthMap;

[[vk::binding(3,0)]]
Sampler2D _normalMap;

[[vk::binding(4,0)]]
Sampler2D _armMap;

[[vk::binding(5,0)]]
ConstantBuffer<GlobalData2> _globalData;

[[vk::binding(6,0)]]
Sampler2D _motionVector;

[[vk::binding(7,0)]]
Sampler2D _previousSampled;

[[vk::binding(8, 0)]]
Sampler2D _albedoMap;

//=====================
// set 1 
[[vk::binding(0,1)]]
StructuredBuffer<ObjDescription> descriptions;

[[vk::binding(1,1)]]
StructuredBuffer<Material> materialValues;

[[vk::binding(2,1)]]
Sampler2D textures[];

[[vk::binding(3,1)]]
RaytracingAccelerationStructure _tlas;

struct ReflectionsParameters{
    bool accumulate;
}

[[vk::push_constant]]
ReflectionsParameters reflectionsParameters;



internal SurfaceGeometry GetSurface(uint32_t primitiveIndex, uint32_t instanceIndex, float2 baricentric, float3x4 objectToWorld)
{
    SurfaceGeometry g;
    const float3 barycentricCoords = float3(1.0f - baricentric.x - baricentric.y, baricentric.x, baricentric.y);

    ObjDescription object = descriptions[NonUniformResourceIndex(instanceIndex)];

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
    
    //================================
    // Texture coordinates 
    g.uv = v1.uv.xy * barycentricCoords.x + v2.uv.xy * barycentricCoords.y + v3.uv.xy * barycentricCoords.z;

    //=================================
    // Normal vector retrieval
    // - wont use the normal map since for reflections it des not matter that much, also it is 1/3 of resolution
    float3 N = v1.norm * barycentricCoords.x + v2.norm * barycentricCoords.y + v3.norm * barycentricCoords.z;
    g.normal = N;

    //=================================
    // Position retrieval
    g.position = v1.pos * barycentricCoords.x + v2.pos * barycentricCoords.y + v3.pos * barycentricCoords.z;    

    //=================================
    // Calculate tangent basis
    float4 tangennt = float4(
        // interpolate tangent vector across the triangle 
        v1.tangent.xyz * barycentricCoords.x + v2.tangent.xyz * barycentricCoords.y + v3.tangent.xyz * barycentricCoords.z ,1.0);  // should we flip the orientation of tangent vector ? 
        
    float handedness = v1.tangent.w * barycentricCoords.x +
    v2.tangent.w * barycentricCoords.y +
    v3.tangent.w * barycentricCoords.z;
        
        
    tangennt.w = handedness;
    g.tangent = tangennt;

    g.TBN = CalculateTBN(g.normal, g.tangent, (float3x3)objectToWorld);
    g.iTBN = transpose(g.TBN);

    //=====================================
    // Retrieve the normal vectors
    
    let features = materialValues[instanceIndex].Features;
    let values = materialValues[instanceIndex].Values;
    
 
    if(features.hasNormalTexture){
        g.shadingNormal = normalize(SampleNormalMap(textures[features.normalTextureIdx],g.uv, g.TBN));
    }else{
        g.shadingNormal = normalize(mul(objectToWorld, float4(N, 0.0)));
    }

    return g;
}

public PBRMaterial GetMaterial(uint32_t instnaceIndex){
    PBRMaterial pbrMat;
    pbrMat.features = materialValues[instnaceIndex].Features;
    pbrMat.values = materialValues[instnaceIndex].Values;
    pbrMat.albedo_map = textures[pbrMat.features.albedoTextureIdx];
    pbrMat.arm_map = textures[pbrMat.features.armTextureIdx];
    pbrMat.emissive_map = textures[pbrMat.features.emissiveTextureIdx];

    return pbrMat;
}

/*
Reflection map will be lerped in the composite pass based on its alpha channel
*/
[[numthreads(16,16,1)]]
[[shader("compute")]]
void computeMain(uint3 threadID : SV_DispatchThreadID){

    float3 throughPut = 1.0;
    
    var parsedData = _globalData.Parse();
    float2 dims;
    _outputImage.GetDimensions(dims.x, dims.y);
    float2 texCoords = (threadID.xy + 0.5) / dims;

     float depht = _depthMap.Sample(texCoords).x;

    if(depht == 1.0){
        _outputImage[threadID.xy] = float4(1.0);
        return;
    }
 
    float3 pos = _positionBuffer.Sample(texCoords).xyz;
    float3 arm = _armMap.Sample(texCoords).xyz;
    float3 N = normalize(_normalMap.Sample(texCoords).xyz);

    float3 wo = normalize(_globalData.cameraPosition.xyz - pos);

    uint pixelID = threadID.x + threadID.y * (uint)dims.x;
    uint u = tea(pixelID, (uint)parsedData.currentFrame);
    
    PBRMaterial mat;
    mat.roughness = arm.x; 
    mat.metallness = arm.y;

    BxDFContext context;
    context.albedo = _albedoMap.Sample(texCoords);
    context.roughness = arm.x;
    context.metallnes = arm.y;
    
    SurfaceGeometry g;
    g.normal = N;
    g.shadingNormal = N;
    g.shadingPosition = pos;
    g.position = pos;
    
    var sampleBrdf = Specular_GGX(context);

    RayDesc ray;
    // we do not need to shoot ray from camear to world sice whe have the closest intersection in the postion buffer
    if(let brdfSample = sampleBrdf.Sample(wo, g, u)){
        
        //==============================================================
        // first bounce, wo - camera ray, wi - constructed from GBuffer 
        ray.Origin = pos;    
        ray.Direction = brdfSample.wi;
        ray.TMin = 0.0001;
        ray.TMax = 1000;

        //================================================y======================
        // Trace ray and determine the reflected colour based on the surface hit 
        RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> result;

	    result.TraceRayInline(_tlas, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);

        result.Proceed();
        
        if(result.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
        {
            var hitG = GetSurface(result.CommittedPrimitiveIndex(), result.CommittedInstanceIndex(), result.CommittedTriangleBarycentrics(), result.CandidateObjectToWorld3x4());
            var hitMat = GetMaterial(result.CommittedInstanceIndex());
            var hitContext = hitMat.Prepare(hitG);
            throughPut = hitContext.albedo.xyz;  
        }
        
        if(reflectionsParameters.accumulate){

            float alpha = 0.2;
            float2 motionVector = _motionVector.Sample(texCoords).xy;
            
            float2 reprojectedCoords = texCoords - motionVector;
            
            float4 reflLast = _previousSampled.Sample(reprojectedCoords);
            
            _outputImage[threadID.xy].xyz = alpha * throughPut + (1.0f - alpha) * reflLast.xyz;
            return;
        }else{
            _outputImage[threadID.xy].xyz = throughPut;
            return; 
        }
        return;

    }

    _outputImage[threadID.xy] = float4(float3(0.0), 1.0);
}