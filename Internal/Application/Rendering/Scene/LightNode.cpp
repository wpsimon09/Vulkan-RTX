//
// Created by wpsimon09 on 26/01/25.
//

#include "LightNode.hpp"

namespace ApplicationCore {
    LightNode::LightNode(std::shared_ptr<StaticMesh> mesh): SceneNode(mesh)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::DirectionalLightNode;
    }
} // ApplicationCore