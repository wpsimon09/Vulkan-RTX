//
// Created by wpsimon09 on 26/11/24.
//

#include "SceneNode.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


namespace ApplicationCore {
    SceneNode::SceneNode(std::shared_ptr<Mesh> mesh): m_transformation(mesh ? mesh->GetTransformations() : &m_localTransformation)
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

    SceneNode::SceneNode(): m_transformation(&m_localTransformation)
    {
        m_parent = nullptr;
        m_mesh = nullptr;
    }

    void SceneNode::AddChild(std::unique_ptr<SceneNode> child)
    {
        if(child)
        {
            m_children.emplace_back(std::move(child));
            m_isParentNode = true;
        }
        else
        {
            Utils::Logger::LogErrorClient("Failed to add child node, child node is nullptr");
        }
    }

    void SceneNode::AddChild(std::shared_ptr<Mesh> child)
    {
        if(child)
        {
            m_children.emplace_back(std::make_unique<SceneNode>(child));
            m_isParentNode = true;
        }
        else
        {
            Utils::Logger::LogErrorClient("Failed to add child node, child node is nullptr");
        }
    }

    std::vector<std::reference_wrapper<SceneNode>> SceneNode::GetChildren()
    {
        return m_children;
    }

    void SceneNode::Update() const
    {
        if(m_parent)
        {
            m_transformation->ComputeModelMatrix(m_parent->m_transformation->GetModelMatrix());
        }else
        {
            m_transformation->ComputeModelMatrix();
        }

        for(auto& child : m_children)
        {
            child->Update();
        }

    }

    void SceneNode::Render(std::vector<VulkanStructs::DrawCallData>& renderingContext) const
    {
        if(m_mesh)
        {
            VulkanStructs::DrawCallData data{};
            data.indexBuffer = m_mesh->m_vertexArray->GetIndexBuffer().GetBuffer();
            data.vertexBuffer = m_mesh->m_vertexArray->GetVertexBuffer().GetBuffer();
            data.modelMatrix = m_transformation->GetModelMatrix();
            data.firstIndex = 1;
            data.indexCount = m_mesh->GetMeshIndexCount();
            renderingContext.emplace_back(data);
        }
    }



} // ApplicationCore