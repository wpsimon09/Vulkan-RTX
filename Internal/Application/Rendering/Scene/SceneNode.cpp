//
// Created by wpsimon09 on 26/11/24.
//

#include "SceneNode.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"


namespace ApplicationCore {
    SceneNode::SceneNode(std::shared_ptr<Mesh> mesh): m_transformation(mesh.get()->GetTransformations())
    {
        if(mesh)
        {
            m_parent = nullptr;
            m_mesh = mesh;
        }else
        {
            Utils::Logger::LogErrorClient("Mesh is nullptr, creating scene node with no mesh assigned to it....");
        }
    }

    void SceneNode::AddChild(std::shared_ptr<SceneNode> child)
    {
        if(child)
        {
            m_children.emplace_back(child);
            m_isParentNode = true;
        }
        else
        {
            Utils::Logger::LogErrorClient("Failed to add child node, child node is nullptr");
        }
    }

    std::vector<std::shared_ptr<SceneNode>>& SceneNode::GetChildren()
    {
        return m_children;
    }

    void SceneNode::Update() const
    {
        if(m_parent)
        {
            m_transformation.ComputeModelMatrix(m_parent->m_transformation.GetModelMatrix());
        }else
        {
            m_transformation.ComputeModelMatrix();
        }

        for(auto& child : m_children)
        {
            child->Update();
        }

    }

    void SceneNode::Render(std::vector<VulkanStructs::DrawCallData>& renderingContext)
    {
        if(m_mesh)
        {
            // populate rendering context data strucutre
        }
    }



} // ApplicationCore