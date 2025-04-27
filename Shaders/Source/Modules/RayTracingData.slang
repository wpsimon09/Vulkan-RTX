module RayTracingData;


public struct ObjDescription {
    public uint64_t vertexAddress;
    public uint64_t indexAddress;
}

public struct Vertex {
    public float3 pos;
    public float3 norm;
    public float2 uv;
}

