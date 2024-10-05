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
    MESH_GEOMETRY_CUBE = "cube",
    MESH_GEOMETRY_SPHERE = "sphere",
    MESH_GEOMETRY_PLANE = "plane",
    MESH_GEOMETRY_POST_PROCESS = "post proecess"
};



#endif //CLIENTENUMS_HPP
