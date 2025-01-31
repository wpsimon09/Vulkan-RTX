//
// Created by wpsimon09 on 26/01/25.
//

#ifndef LIGHTNODE_HPP
#define LIGHTNODE_HPP
#include "SceneNode.hpp"

namespace ApplicationCore {

class DirectionLightNode: public SceneNode {
public:
    DirectionLightNode(std::shared_ptr<StaticMesh> mesh);
private:
};

} // ApplicationCore

#endif //LIGHTNODE_HPP
