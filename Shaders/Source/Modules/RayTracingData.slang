module RayTracingData;
import Material;

[[vk::binding(2, 0)]]
StructuredBuffer<ObjDescription> descriptions;

public struct ObjDescription {
    ConstBufferPointer<Vertex> vertexAddress;
    ConstBufferPointer<int> indexAddress;
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
    //=================================================
    // for each vertex there is one index that can
    // be used to look up the correct vertex in the array
    int i1 = object.indexAddress[(primitiveIndex * 3)    ]; 
    int i2 = object.indexAddress[(primitiveIndex * 3) + 1];
    int i3 = object.indexAddress[(primitiveIndex) + 2];

    Vertex v1 = object.vertexAddress[i1];
    Vertex v2 = object.vertexAddress[i2];
    Vertex v3 = object.vertexAddress[i3];

    SurfaceGeometry g;
    // get normal of triangle from 3 vertex positions and baricentrics of the current hit point 
    g.normal = v1.norm.xyz * barycentricCoords.x + v2.norm.xyz * barycentricCoords.y + v3.norm.xyz * barycentricCoords.z;
    return g;
}


