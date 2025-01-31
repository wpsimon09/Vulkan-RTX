//
// Created by wpsimon09 on 26/01/25.
//

#include "DirectionLightNode.hpp"

namespace ApplicationCore {
    DirectionLightNode::DirectionLightNode(std::shared_ptr<StaticMesh> mesh): SceneNode(mesh)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::DirectionalLightNode;
    }
} // ApplicationCore