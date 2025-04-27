module RayTracingData;
import Surface;

[[vk::binding(2, 0)]]
StructuredBuffer<ObjDescription> descriptions;

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
    g.normal = normalize(v1.norm * barycentricCoords.x + v2.norm * barycentricCoords.y + v3.norm * barycentricCoords.z);

    return g;
}


