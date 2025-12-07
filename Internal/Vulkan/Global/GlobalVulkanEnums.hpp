//
// Created by wpsimon09 on 06/10/24.
//

#ifndef GLOBALVULKANENUMS_HPP
#define GLOBALVULKANENUMS_HPP

enum class EPipelineType
{
    RasterPBRTextured = 0,
    Transparent,
    RTX,
    DebugLines,
    Outline,
    MultiLight,
    EditorBillboard,
    DebugShadpes
};

enum EQueueFamilyIndexType
{
    Graphics = 0,
    Transfer,
    PresentKHR,
    Compute
};

enum EBufferType
{
    Vertex = 0,
    VertexStaging,
    Image,
    ImageStaging,
    Index,
    IndexStaging,
    Undefined
};

enum EImageSource
{
    File = 0,
    Buffer,
    Generated
};

enum EVertexInput
{
    Full = 0,
    PositionOnly,
    Position_UV,
    Position_Normal,
    Position_Normal_Uv,
    Empty
};

enum EShaderBindingGroup
{
    ForwardLit,
    ForwardUnlit,
    ForwardUnlitNoMaterial,
    Debug,
    NullBinding,
    Skybox,
    RayTracing,
    ShadowRT,
    ToneMap,
    PostProcessing,
    FogBinding,
    LensFlareBinding,
    ComputePostProecess,
    LuminanceHistrogram,
    AverageLuminance
};

enum EFrameStages
{
    Uninitialized = 0,
    TransferFinish,
    RenderFinish,
    SafeToBegin,
    NumStages
};


#endif  //GLOBALVULKANENUMS_HPP