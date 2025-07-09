//=================================
// Structs that are the core functionality of shaders
// global data, per object data
module Core;

public struct GlobalData {
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
public struct PostProcessingParameters{
    public float4 toneMapping; // x - exposure, y - gamma correction, zw - padding
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