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
    context.albedo = float4(1.0);
    context.roughness = arm.x;
    context.metallnes = arm.y;

    SurfaceGeometry g;
    g.normal = N;
    g.shadingNormal = N;
    g.shadingPosition = pos;
    g.position = pos;
    
    var sample = mat.Sample(context, wo, g, u);
    
    RayDesc ray;
    // we do not need to shoot ray from camear to world sice whe have the closest intersection in the postion buffer
    if(let sample = sample.brdf.Sample(wo, g, u)){
        ray.Origin = pos;    
        ray.Direction = sample.wi;
        ray.TMin = 0.0001;
        ray.TMax = 1000;

        //======================================================================
        // Trace ray and determine the reflected colour based on the surface hit 
        RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> result;

	    result.TraceRayInline(_tlas, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);

        result.Proceed();
        //var result = TraceRayInline(ray, _tlas, u);
        float4 reflectedColour = 0.0;
        if(result.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
        {
            var surface = GetSurface(result.CommittedPrimitiveIndex(), result.CommittedInstanceIndex(), result.CommittedTriangleBarycentrics(), result.CandidateObjectToWorld3x4());

            let features = materialValues[NonUniformResourceIndex(result.CommittedInstanceIndex())].Features;
            let values = materialValues[NonUniformResourceIndex(result.CommittedInstanceIndex())].Values;

            var mat = GetMaterial(result.CommittedInstanceID());
            var brdfContext = mat.Prepare(surface);
            var brdf = mat.Sample(brdfContext, wo, g, u);
            
            if (let brdfSample = brdf.brdf.Sample(wo, g, u)){

                float3 col = (brdf.brdf.Evaluate(wo, brdfSample.wi, g) * dot(normalize(wo), normalize(g.normal))) / sample.PDF;

                float alpha = 0.3;
                float2 motionVector = _motionVector.Sample(texCoords).xy;
            
                float2 reprojectedCoords = texCoords - motionVector;
                
                float4 reflLast = _previousSampled.Sample(reprojectedCoords);
                
                reflectedColour.xyz = alpha * reflectedColour.xyz + (1.0f - alpha) * reflLast.xyz;
            }
            _outputImage[threadID.xy] += reflectedColour;

            return;
        }

    }

    _outputImage[threadID.xy] = float4(float3(0.0), 1.0);
}