module RayTracingData;
import Surface;
import Material;

[[vk::binding(2, 0)]]
StructuredBuffer<ObjDescription> descriptions;

[[vk::binding(5, 0)]]
StructuredBuffer<Material, Std430DataLayout> materialValues;

public struct ObjDescription {
    public Vertex* vertexAddresses;
    public int* indexAddress;
}

public struct Vertex {
    public float3 pos;
    public float3 norm;
    public float2 uv;
}

public SurfaceGeometry GetSurfaceInfo(uint32_t primitiveIndex, uint32_t instanceIndex, float2 baricentric)
{
    const float3 barycentricCoords = float3(1.0f - baricentric.x - baricentric.y, baricentric.x, baricentric.y);

    ObjDescription object = descriptions[instanceIndex];
    Material mat = materialValues[instanceIndex];

    // Vertex *vertexBuffer = (Vertex *)(object.vertexAddresses); // Cast to Vertex pointer
    // int *indexBuffer = (int *)(object.indexAddress);        // Cast to int pointer (assuming indices are integers)

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

    // Calculate normal using barycentric interpolation
    SurfaceGeometry g;
    float3 N = v1.norm * barycentricCoords.x + v2.norm * barycentricCoords.y + v3.norm * barycentricCoords.z;
    g.normal = N;
    g.shadingNormal = normalize(mul(ObjectToWorld3x4(), float4(N,1.0)));

    float3 pos = v1.pos * barycentricCoords.x + v2.pos * barycentricCoords.y + v3.pos * barycentricCoords.z;
    g.position = pos;
    g.shadingPosition = mul(ObjectToWorld(), float4(pos, 1.0)).xyz;

    float2 uv = v1.uv * baricentric.x + v2.uv * baricentric.y;

    return g;
}

public Material GetMaterial(uint32_t instanceIndex) {
    Material mat;
    mat.Features = materialValues[instanceIndex].Features;
    mat.Values = materialValues[instanceIndex].Values;
    return mat;
}


