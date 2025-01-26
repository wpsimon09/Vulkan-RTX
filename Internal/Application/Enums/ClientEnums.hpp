//
// Created by wpsimon09 on 04/10/24.
//

#ifndef CLIENTENUMS_HPP
#define CLIENTENUMS_HPP

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
    Cube ,
    Sphere ,
    Plane ,
    PostProcessQuad,
    Triangle,
    Custom,
    Cross
};

enum ETextureType
{
    Diffues = 0,
    arm ,
    normal,
    emissive,
};




#endif //CLIENTENUMS_HPP
