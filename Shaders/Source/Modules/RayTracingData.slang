module RayTracingData;
import Surface;
import Material;
import Utils;

[[vk::binding(2, 0)]]
StructuredBuffer<ObjDescription> descriptions;

[[vk::binding(5, 0)]]
StructuredBuffer<Material> materialValues;

[[vk::binding(7, 0)]]
Sampler2D textures[];

public struct ObjDescription {
    public Vertex* vertexAddresses;
    public int* indexAddress;
}

public struct Vertex {
    public float3 pos;
    public float3 norm;
    public float2 uv;
    public float4 tangent; 
}

public SurfaceGeometry GetSurfaceInfo(uint32_t primitiveIndex, uint32_t instanceIndex, float2 baricentric)
{
    const float3 barycentricCoords = float3(1.0f - baricentric.x - baricentric.y, baricentric.x, baricentric.y);

    ObjDescription object = descriptions[instanceIndex];

    // //=================================================
    // // for each vertex there is one index that can
    // // be used to look up the correct vertex in the array
    int i1 = object.indexAddress[primitiveIndex * 3];     // First index
    int i2 = object.indexAddress[primitiveIndex * 3 + 1]; // Second index
    int i3 = object.indexAddress[primitiveIndex * 3 + 2]; // Third index

    // // Fetch the corresponding vertices
    Vertex v1 = object.vertexAddresses[i1];
    Vertex v2 = object.vertexAddresses[i2];
    Vertex v3 = object.vertexAddresses[i3];

    let features = materialValues[instanceIndex].Features;
    let values = materialValues[instanceIndex].Values;
    
    // Calculate normal using barycentric interpolation
    SurfaceGeometry g;
    float3 N = v1.norm * barycentricCoords.x + v2.norm * barycentricCoords.y + v3.norm * barycentricCoords.z;
    g.normal = N;
    g.shadingNormal = mul(ObjectToWorld(), float4(N, 0.0));

    
    float3 pos = v1.pos * barycentricCoords.x + v2.pos * barycentricCoords.y + v3.pos * barycentricCoords.z;
    g.position = pos;
    g.shadingPosition = mul(ObjectToWorld(), float4(pos, 1.0)).xyz;
    
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

    

    g.TBN = CalculateTBN(g.normal, g.tangent, (float3x3)ObjectToWorld());
    g.iTBN = transpose((g.TBN));

    if(features.hasNormalTexture){
        g.shadingNormal = normalize(SampleNormalMap(textures[features.normalTextureIdx],g.uv, g.TBN));
    }else{
        g.shadingNormal = normalize(mul(ObjectToWorld(), float4(N, 0.0)));
    }
    
    return g;
}
    
public CookTorrence_GGX GetMaterial(uint32_t instanceIndex, SurfaceGeometry g) {
    PBRMaterial pbrMat;
    pbrMat.features = materialValues[instanceIndex].Features;
    pbrMat.values = materialValues[instanceIndex].Values;
    pbrMat.albedo_map = textures[pbrMat.features.albedoTextureIdx];
    pbrMat.arm_map = textures[pbrMat.features.armTextureIdx];
    pbrMat.emissive_map = textures[pbrMat.features.emissiveTextureIdx];

    return pbrMat.Prepare(g);
}
