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
        mesh ? mesh->GetTransformations() : &m_localTransformation), m_sceneNodeMetaData{}
    {
        if (mesh)
        {
            m_parent = nullptr;
            m_mesh = mesh;
            m_sceneNodeMetaData.HasMesh = true;
            m_sceneNodeMetaData.ID = ++SceneNodeIDCounter;
        }
        else
        {
            Utils::Logger::LogErrorClient("Mesh is nullptr, creating scene node with no mesh assigned to it....");
        }
    }

    SceneNode::SceneNode(): m_transformation(&m_localTransformation), m_sceneNodeMetaData{}
    {
        m_parent = nullptr;
        m_mesh = nullptr;
        m_sceneNodeMetaData.ID = ++SceneNodeIDCounter;
    }

    void SceneNode::AddChild(const std::shared_ptr<SceneNode>& child)
    {
        if (child)
        {
            m_children.emplace_back(child);
            child->m_parent = this;
            m_sceneNodeMetaData.IsParentNode = true;
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
            m_sceneNodeMetaData.IsParentNode = true;
        }
        else
        {
            Utils::Logger::LogErrorClient("Failed to add child node, child node is nullptr");
        }
    }

    void SceneNode::Setvisibility(bool isVisible)
    {
        m_sceneNodeMetaData.IsVisible = isVisible;
        for (auto& child : m_children)
        {
            child->Setvisibility(isVisible);
        }
    }


    void SceneNode::Select(bool selectedFromWorld)
    {

        m_sceneNodeMetaData.IsSelected = true;
        m_sceneNodeMetaData.IsSelectedFromWorld = selectedFromWorld;
        UpdateParentsAboutChildStatus(true, m_parent);

        for (auto& child : m_children)
        {
            child->Select(selectedFromWorld);
        }
    }

    void SceneNode::Deselect()
    {

        m_sceneNodeMetaData.IsSelected = false;
        UpdateParentsAboutChildStatus(false, m_parent);

        for (auto& child : m_children)
        {
            child->Deselect();
        }
    }

    void SceneNode::PreformRayIntersectionTest(Ray& ray,std::vector<std::shared_ptr<SceneNode>>& result)
    {
        if (m_sceneNodeMetaData.HasMesh && m_sceneNodeMetaData.IsVisible)
        {
            // transfer bounds max a    nd min to world space
            m_mesh->GetMeshData()->bounds.ProjectToWorld(m_transformation->GetModelMatrix());
            if (ApplicationCore::AABBRayIntersection(ray, &m_mesh->GetMeshData()->bounds))
            {
                if (!m_sceneNodeMetaData.IsSelected)
                {
                    m_sceneNodeMetaData.IsSelected = true;
                }
                result.emplace_back(shared_from_this());
            }else
            {
                Deselect();
            }
        }
        for (auto& child : m_children) {
            child->PreformRayIntersectionTest(ray, result);
        }
    }

    SceneNode* SceneNode::GetParent()
    {
        return m_parent;
    }

    std::vector<std::reference_wrapper<SceneNode>> SceneNode::GetChildrenByWrapper()
    {
        std::vector<std::reference_wrapper<SceneNode>> result;
        for (auto& child : m_children)
        {
            result.emplace_back(std::ref(*child));
        }

        return result;
    }

    void SceneNode::UpdateParentsAboutChildStatus(bool status, SceneNode* parent)
    {
        if (parent ==nullptr)
            return;
        if (parent->GetParent())
        {
            parent->m_sceneNodeMetaData.IsAnyChildSelected = status;
            auto nextParent = parent->GetParent();
            UpdateParentsAboutChildStatus(status, nextParent);
        }
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
        if (m_mesh && m_sceneNodeMetaData.IsVisible)
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
                    VulkanStructs::DrawCallData data;
                    data.modelMatrix = m_transformation->GetModelMatrix();
                    data.firstIndex = 1;
                    data.indexCount = m_mesh->GetMeshIndexCount();
                    data.indexCount_BB = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);
                    data.material = m_mesh->m_material;
                    data.meshData = m_mesh->GetMeshData();
                    data.renderOutline = m_sceneNodeMetaData.IsSelected;
                    data.position = m_transformation->GetPosition();

                    //=====================================================
                    // BOUNDING VOLUME STUFF
                    //=====================================================
                    data.bounds = &m_mesh->GetMeshData()->bounds;

                    //=====================================================
                    // SORT BASED ON THE DEPTH
                    //=====================================================
                    renderingContext->DrawCalls.emplace_back(data);

                }
            }
        }
    }
} // ApplicationCore
