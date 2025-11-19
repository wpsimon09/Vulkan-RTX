//=================================
// Structs that are the core functionality of shaders
// global data, per object data
module Core;
import Surface;
import BxDF;

public enum ERenderOutput{
    FullLit = 0,
    NormalVectors,
    LightOnly,
    Albedo,
    Arm,
};

public enum ERenderOutputRTX{
    PathTraced = 0,
    Normals,
    PDF,
};

public enum ECameraType{
    PinHole = 0, 
    ThinLens
};

public struct GlobalData {
    public column_major float4x4 view;
    public column_major float4x4 projection;
    public column_major float4x4 inverseView;
    public column_major float4x4 invProjection;

    public float4 atmosphereInfo; // x - bottom atmosphere radius, y - up atmosphere radius, zw - padding
    public float4 cameraPostiion; // xyz - camera position, w - fov in radians  
    public float4 viewParams; // x - defocuse strength, y - focal plane pos, z - nerPlane, w - far plane
    public float4 viewParams2;  // x - FOV, y - aspect ratio, zw -camera type

    public float raysPerPixel;
    public float reccursionDepth;
    public float2 screenSize;

    public float currentFrame;
    public ERenderOutput renderOutput; 
    public ERenderOutputRTX renderOutputRTX;

    public bool accumulateFrames;
    public bool aoOcclusion;
    public bool isRayTracing;
    public bool accountForSunTransmitance;
};


public struct PermodelData {
    public float4x4 modelMatrix;
    public float4x4 normalMatrix;
    public float4 positionWorldSpace;
    public uint4 indexes; // x - material index;
    
};

//=================================================
// push constant passed every frame 
// used to index material buffer per object buffer
public struct PerObjectPushConstatn {
    public int4 perObjectDataIndex; //x- index to the the material yzw - other indexes
    public column_major float4x4 modelMatrix;
}

//===============================================
// parameters to controll tone mapping 
public struct ToneMappingParameters{
    public float whitePoint;
    public float gamma;
    public bool isRayTracing;
    public float curve;
}

//===============================================
// parameters to controll lens flare effect
public struct LensFlareParameters
{
    public float     lensFlareStrength;
    public float     f1Strength       ;
    public float     f2Strength       ;
    public float     f3Strength       ;
}

//================================================
// parameters for fog rendering
public struct FogVolumeParameters {
    public float sigma_a; // absortpion coeficient
    public float sigma_s; // scattering coeficient
    public float rayDistance;
    public float raySteps;

    public float4 fogColour;
    
    public float heightFallOff;
    public bool rayMarched;
    public float asymetryFactor; // G - for the phase function
    public float fogHeight;
}

//==========================
// Ray hit struct for the ray query
public struct HitInfo{
    bool hit = false;
    float t = -1.0;
}

public struct RayPayLoad {
    public Optional<SurfaceGeometry> hitSurface = {};

    public float3 throughput = 1.0;
    public float3 radiance = 0.0;

    public float3 rayDirection = {};
    public float3 rayOrigin = {};

    public uint depth = 0;
    public uint randomContext;

    public Optional<BxDFContext> brdfContext;
}