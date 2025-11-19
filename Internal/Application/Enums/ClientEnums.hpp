//
// Created by wpsimon09 on 04/10/24.
//

#ifndef CLIENTENUMS_HPP
#define CLIENTENUMS_HPP

#include "string"

enum EPrimitiveTopology
{
    //points
    PointList,

    //lines
    LineList,
    LineStrip,

    //triangles
    TriangleList,
    TriangleStrip,
    TriangleFan,

    //patch (tesselation only)
    Patch
};

enum EMeshGeometryType
{
    Cube,
    Sphere,
    Plane,
    PostProcessQuad,
    Triangle,
    Custom,
    Cross,
    Arrow,
    LinePlane,
    SkyBox
};

enum ETextureType
{
    Diffues = 0,
    arm,
    normal,
    Emissive,
};

enum EEditorIcon
{
    PointLight = 0,
    DirectionalLight,
    SpotLigth,
    AreaLight
};

enum ENodeType
{
    Node = 0,
    MeshNode,
    DirectionalLightNode,
    PointLightNode,
    SpotLightNode,
    AreaLightNode,
    SkyBoxNode,
    FogVolume,
    Atmosphere,
};

inline std::string IconToString(EEditorIcon icon)
{
    switch(icon)
    {
        case PointLight:
            return "Point Light";
        case DirectionalLight:
            return "Directional Light";
        case SpotLigth:
            return "Spot Light";
        case AreaLight:
            return "Area Light";
        default:
            return "Unknown";
    }
}

enum ETheme
{
    Light = 0,
    Dark
};


enum EDebugRendering
{
    Lit = 0,
    NormalVectors,
    LightOnly,
    Albedo,
    Arm,
};

enum EDebugRenderingRTX
{
    PathTraced = 0,
    Normals,
    PDF
};

enum EToneMappingCurve
{
    Aces_Approc = 0,
    Uchimura,
    Aces_Full
};

enum ECameraTypes
{
    Pinhole = 0,
    ThinLens
};

#endif  //CLIENTENUMS_HPP
