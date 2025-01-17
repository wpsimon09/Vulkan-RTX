//
// Created by wpsimon09 on 04/10/24.
//

#ifndef CLIENTENUMS_HPP
#define CLIENTENUMS_HPP

enum PRIMITIVE_TOPOLOGY
{
    //points
    TOPOLOGY_POINT_LIST,

    //lines
    TOPOLOGY_LINE_LIST,
    TOPOLOGY_LINE_STRIP,

    //triangles
    TOPOLOGY_TRIANGLE_LIST,
    TOPOLOGY_TRIANGLE_STRIP,
    TOPOLOGY_TRIANGLE_FAN,

    //patch (tesselation only)
    TOPOLOGY_PATCH
};

enum MESH_GEOMETRY_TYPE
{
    MESH_GEOMETRY_CUBE ,
    MESH_GEOMETRY_SPHERE ,
    MESH_GEOMETRY_PLANE ,
    MESH_GEOMETRY_POST_PROCESS,
    MESH_GEOMETRY_TRIANGLE,
    MESH_GEOMETRY_CUSTOM,
    MESH_GEOMETRY_CROSS
};

enum ETextureType
{
    Diffues = 0,
    arm ,
    normal,
    emissive,
};




#endif //CLIENTENUMS_HPP
