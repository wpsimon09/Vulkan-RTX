//
// Created by wpsimon09 on 26/01/25.
//

#include "DirectionLightNode.hpp"

#include "Vulkan/Global/VulkanStructs.hpp"

namespace ApplicationCore {
    DirectionLightNode::DirectionLightNode(std::shared_ptr<StaticMesh> mesh): SceneNode(mesh), m_parameters()
    {
        m_sceneNodeMetaData.nodeType = ENodeType::DirectionalLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bMainLightPass = false;



    }

    void DirectionLightNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        SceneNode::Render(renderingContext);
    }
} // ApplicationCore