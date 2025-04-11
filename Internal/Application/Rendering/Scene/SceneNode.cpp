//
// Created by wpsimon09 on 26/11/24.
//

#include "SceneNode.hpp"

#include <GLFW/glfw3.h>

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/IntersectionTests/IntersectionTests.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/EngineOptions.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


namespace ApplicationCore
{
    SceneNode::SceneNode(std::shared_ptr<StaticMesh> mesh): m_transformation(
        &m_localTransformation), m_sceneNodeMetaData{}
    {
        if (mesh)
        {
            m_parent = nullptr;
            m_mesh = mesh;
            m_sceneNodeMetaData.HasMesh = true;
            m_sceneNodeMetaData.ID = ++SceneNodeIDCounter;
            m_sceneNodeMetaData.nodeType = ENodeType::MeshNode;


        }
        else
        {
            Utils::Logger::LogErrorClient("Mesh is nullptr, creating scene node with no mesh assigned to it....");
        }
    }

    SceneNode::SceneNode(SceneNode& other):m_transformation(&m_localTransformation)
    {
        m_parent = nullptr;

        if (other.m_mesh != nullptr){
            m_mesh = std::make_shared<ApplicationCore::StaticMesh>(*other.GetMesh()->GetMeshData(), other.GetMesh()->GetMaterial());
        }
        m_localTransformation = Transformations();
        m_sceneNodeMetaData = other.m_sceneNodeMetaData;
        m_name = other.m_name + "##" + VulkanUtils::random_string(4);
        m_sceneNodeMetaData.ID = ++SceneNodeIDCounter;

        for (auto& child : other.m_children)
        {
            AddChild(std::make_shared<SceneNode>(*child));
        }
    }


    SceneNode::SceneNode(): m_transformation(&m_localTransformation)
    {
        m_localTransformation = Transformations();
        m_parent = nullptr;
        m_mesh = nullptr;
        m_sceneNodeMetaData.ID = ++SceneNodeIDCounter;
        // `scene node` node type is defautl
    }

    void SceneNode::AddChild(const std::shared_ptr<SceneNode>& child)
    {
        if (child != nullptr)
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

    void SceneNode::AddChild(std::shared_ptr<StaticMesh> child)
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
        for (auto& child : m_children) {
            child->Setvisibility(isVisible);
        }
    }


    void SceneNode::Select(bool selectedFromWorld)
    {

        m_sceneNodeMetaData.IsSelected = true;
        m_sceneNodeMetaData.IsSelectedFromWorld = selectedFromWorld;

        for (auto& child : m_children) {
            child->Select(selectedFromWorld);
        }
    }

    void SceneNode::Deselect()
    {
        m_sceneNodeMetaData.IsSelected = false;
        for (auto& child : m_children) {
            child->Deselect();
        }
    }

    void SceneNode::PreformRayIntersectionTest(Ray& ray,std::vector<std::shared_ptr<SceneNode>>& result)
    {
        if (m_sceneNodeMetaData.HasMesh && m_sceneNodeMetaData.IsVisible)
        {
            // transfer bounds max and min to world space
            m_mesh->GetMeshData()->bounds.ProjectToWorld(m_transformation->GetModelMatrix());
            if (ApplicationCore::AABBRayIntersection(ray, &m_mesh->GetMeshData()->bounds)) {
                result.emplace_back(shared_from_this());
            }
        }
        for (auto& child : m_children) {
            child->PreformRayIntersectionTest(ray, result);
        }
    }

    float SceneNode::GetDistanceFromCamera(glm::vec3 cameraPosition)
    {
        if (m_mesh){
            m_mesh->GetMeshData()->bounds.ProjectToWorld(m_transformation->GetModelMatrix());
            glm::vec3 pos = m_mesh->GetMeshData()->bounds.origin;

            return glm::length(pos - cameraPosition);
        }return 20000.f;
    }

    bool SceneNode::IsLight() const
    {
        return m_sceneNodeMetaData.nodeType == ENodeType::DirectionalLightNode
        || m_sceneNodeMetaData.nodeType == ENodeType::PointLightNode
        || m_sceneNodeMetaData.nodeType == ENodeType::SpotLightNode
        || m_sceneNodeMetaData.nodeType == ENodeType::AreaLightNode
        || m_sceneNodeMetaData.nodeType == ENodeType::SkyBoxNode;
    }

    SceneNode* SceneNode::GetParent()
    {
        return m_parent;
    }

    std::vector<std::reference_wrapper<SceneNode>> SceneNode::GetChildrenByWrapper()
    {
        std::vector<std::reference_wrapper<SceneNode>> result;
        for (auto& child : m_children) {
            result.emplace_back(std::ref(*child));
        }

        return result;
    }

    void SceneNode::Update()
    {
        if (m_parent) {
            m_transformation->ComputeModelMatrix(m_parent->m_transformation->GetModelMatrix());
        }
        else {
            m_transformation->ComputeModelMatrix();
        }

        for (auto& child : m_children)
        {
            child->Update();
        }
    }

    void SceneNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
    {
        if (m_mesh && m_sceneNodeMetaData.IsVisible)
        {

            // frustrum culling
            if (m_sceneNodeMetaData.FrustumCull && GlobalVariables::RenderingOptions::EnableFrustrumCulling)
            {
                if (!VulkanUtils::IsInViewFrustum(
                            &m_mesh->GetMeshData()->bounds,
                        m_transformation->GetModelMatrix(),
                        renderingContext->view, renderingContext->projection)) {
                    return;
                }
            }

            //=====================================================
            // NORMAL SCENE DATA
            //=====================================================
            ;

            VulkanStructs::DrawCallData data;
            data.firstIndex = 1;

            data.indexCount = m_mesh->GetMeshIndexCount();
           // data.indexCount_BB = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);

            data.bounds = &m_mesh->GetMeshData()->bounds;
            data.vertexData = &m_mesh->GetMeshData()->vertexData;
            data.indexData = &m_mesh->GetMeshData()->indexData;

            data.modelMatrix = m_transformation->GetModelMatrix();
            if (renderingContext->WireFrameRendering)
            {
                data.effect = effectsLibrary.GetEffect(EEffectType::DebugLine);
            }
            else if (m_mesh->m_currentMaterial->IsTransparent())
            {
                data.inDepthPrePass = false;
                data.effect = effectsLibrary.GetEffect(EEffectType::AplhaBlend);
            }
            else
            {
                //data.effect = effectsLibrary.GetEffect(EEffectType::ForwardShader);
                data.effect = m_mesh->GetMaterial()->GetEffect();

            }

            data.position = m_transformation->GetPosition();
            data.bounds = &m_mesh->GetMeshData()->bounds;
            data.material = m_mesh->m_currentMaterial.get();

            if (m_sceneNodeMetaData.IsSelected) data.selected = true;
            renderingContext->AddDrawCall(data);

            if (m_sceneNodeMetaData.IsSelected)
            {
                data.inDepthPrePass = false;

                data.effect = effectsLibrary.GetEffect(EEffectType::Outline);
                data.modelMatrix *= glm::scale(glm::mat4(1.0f), glm::vec3(1.0f + GlobalVariables::RenderingOptions::OutlineWidth));
                renderingContext->AddDrawCall(data);
            }

            //=====================================================
            // BOUNDING VOLUME STUFF
            //=====================================================
            if (renderingContext->RenderAABB)
            {
                data.vertexData = &m_mesh->GetMeshData()->vertexData_BB;
                data.indexData = &m_mesh->GetMeshData()->indexData_BB;
                data.indexCount = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);;
                data.effect = effectsLibrary.GetEffect(EEffectType::DebugLine);
                renderingContext->AddDrawCall(data);
            }

            if (m_sceneNodeMetaData.CastsShadows)
            {
                // change effect
            }

            // if in ray tracing capture do some shenanigans here
        }
    }
} // ApplicationCore
