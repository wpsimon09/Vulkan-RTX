module Surface;

//=================================
// STRUCTS
//=================================
public struct SurfaceGeometry
{
    public float3 position;
    public float3 shadingPosition;

    public float3 normal; // gemetry normal
    public float3 shadingNormal;
    
    public float4 tangent;
    
    public float2 uv;
}