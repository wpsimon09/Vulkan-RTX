//
// Created by wpsimon09 on 26/11/24.
//

#include "SceneNode.hpp"

#include <GLFW/glfw3.h>

#include "Application/IntersectionTests/IntersectionTests.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


namespace ApplicationCore
{
    SceneNode::SceneNode(std::shared_ptr<Mesh> mesh): m_transformation(
        mesh ? mesh->GetTransformations() : &m_localTransformation)
    {
        if (mesh)
        {
            m_parent = nullptr;
            m_mesh = mesh;
            m_hasMesh = true;
            m_ID = ++SceneNodeIDCounter;
        }
        else
        {
            Utils::Logger::LogErrorClient("Mesh is nullptr, creating scene node with no mesh assigned to it....");
        }
    }

    SceneNode::SceneNode(): m_transformation(&m_localTransformation)
    {
        m_parent = nullptr;
        m_mesh = nullptr;
        m_ID = ++SceneNodeIDCounter;
    }

    void SceneNode::AddChild(const std::shared_ptr<SceneNode>& child)
    {
        if (child)
        {
            m_children.emplace_back(child);
            child->m_parent = this;
            m_isParentNode = true;
        }
        else
        {
            Utils::Logger::LogErrorClient("Failed to add child node, child node is nullptr");
        }
    }

    void SceneNode::AddChild(std::shared_ptr<Mesh> child)
    {
        if (child)
        {
            auto newNode = std::make_shared<SceneNode>(child);
            newNode->m_parent = this;
            m_children.emplace_back(newNode);
            m_isParentNode = true;
        }
        else
        {
            Utils::Logger::LogErrorClient("Failed to add child node, child node is nullptr");
        }
    }

    void SceneNode::Setvisibility(bool isVisible)
    {
        m_isVisible = isVisible;
        for (auto& child : m_children)
        {
            child->Setvisibility(isVisible);
        }
    }

    void SceneNode::Select()
    {
        if (m_name == "Root-Node")
            return;
        m_isSelected = true;
        for (auto& child : m_children)
        {
            child->Select();
        }
    }

    void SceneNode::Deselect()
    {
        if (m_name == "Root-Node")
            return;
        m_isSelected = false;
        for (auto& child : m_children)
        {
            child->Deselect();
        }
    }

    void SceneNode::PreformRayIntersectionTest(Ray& ray)
    {
        if (m_hasMesh)
        {
            // transfer bounds max and min to world space
            VulkanStructs::Bounds* bounds = &m_mesh->GetMeshData()->bounds;
            bounds->ProjectToWorld(m_transformation->GetModelMatrix());
            if (ApplicationCore::AABBRayIntersection(ray, bounds))
            {
                Select();
                return;
            }
        }
        for (auto& child : m_children) {
            child->PreformRayIntersectionTest(ray);
        }
    }

    SceneNode* SceneNode::GetParent()
    {
        return m_parent;
    }

    std::vector<std::reference_wrapper<SceneNode>> SceneNode::GetChildren()
    {
        std::vector<std::reference_wrapper<SceneNode>> result;
        for (auto& child : m_children)
        {
            result.emplace_back(std::ref(*child));
        }

        return result;
    }

    void SceneNode::Update() const
    {
        if (m_parent)
        {
            m_transformation->ComputeModelMatrix(m_parent->m_transformation->GetModelMatrix());
        }
        else
        {
            m_transformation->ComputeModelMatrix();
        }

        for (auto& child : m_children)
        {
            child->Update();
        }
    }

    void SceneNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        if (m_mesh && m_isVisible)
        {
            // check if the mesh can be rendered in the given context
            if (m_mesh->GetRenderingMetaData() == renderingContext->metaData)
            {
                // frustrum culling
                if (VulkanUtils::IsInViewFrustum(
                    &m_mesh->GetMeshData()->bounds,
                    m_transformation->GetModelMatrix(),
                    renderingContext->metaData.view, renderingContext->metaData.projection))
                {
                    //=====================================================
                    // NORMAL SCENE DATA
                    //=====================================================
                    VulkanStructs::DrawCallData data{.modelMatrix = m_transformation->GetModelMatrix()};
                    data.firstIndex = 1;
                    data.indexCount = m_mesh->GetMeshIndexCount();
                    data.indexCount_BB = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);
                    data.material = m_mesh->m_material;
                    data.meshData = m_mesh->GetMeshData();
                    data.renderOutline = m_isSelected;

                    //=====================================================
                    // BOUNDING VOLUME STUFF
                    //=====================================================
                    data.bounds = &m_mesh->GetMeshData()->bounds;

                    renderingContext->DrawCalls.emplace_back(data);
                }
            }
        }
    }
} // ApplicationCore
