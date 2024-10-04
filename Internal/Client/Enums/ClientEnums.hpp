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


#endif //CLIENTENUMS_HPP
