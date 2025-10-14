//=================================
// Structs that are the core functionality of shaders
// global data, per object data
module Core;

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
}

public struct GlobalData {
    public column_major float4x4 view;
    public column_major float4x4 projection;
    public column_major float4x4 inverseView;
    public column_major float4x4 invProjection;

    public float4 atmosphereInfo; // x - bottom atmosphere radius, y - up atmosphere radius, zw - padding
    public float4 cameraPostiion; // xyz - camera position, w - bottom atmosphere radius
    public float4 viewParams; // x - defocuse strength, y - focal plane pos, z - nerPlane, w - far plane
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



//===============================================================
// same as global data but with meanigfull names for ray tracing 
public struct GlobalDataRTX {
    public column_major float4x4 view;
    public column_major float4x4 projection;
    public column_major float4x4 inverseView;
    public column_major float4x4 invProjection;

    public float4 lightPosition;
    public float4 cameraPostiion;
    public float4 viewParams; // xy- camera plane width and height,z - nerPlane, w - focal length

    public float raysPerPixel;
    public float reccursionDepth;
    public float2 screenSize;

    public float currentFrame;
    public ERenderOutput renderOutput;
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
    public float3 distance;
    public float3 normal;
    public float3 reflector;

    public float3 incomingLightColour;
    public float3 rayColour;

    public float3 rayDirection;
    public float3 rayOrigin;

    public float PDF = 0;  
    public uint depth;
    public uint randomContext;

    public bool hasHit;
}