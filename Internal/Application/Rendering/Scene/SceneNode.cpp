//
// Created by wpsimon09 on 26/11/24.
//

#include "SceneNode.hpp"

#include <GLFW/glfw3.h>

#include "Application/IntersectionTests/IntersectionTests.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/EngineOptions.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
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
            if(mesh->GetMaterial()->IsTransparent()){
                m_sceneNodeMetaData.RenderingMetaData.bTransparentPass = true;
                m_sceneNodeMetaData.RenderingMetaData.bOpaquePass = false;
            }

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
        || m_sceneNodeMetaData.nodeType == ENodeType::AreaLightNode;
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

        if(m_mesh){
            if(m_mesh->GetMaterial()->IsTransparent()){
                m_sceneNodeMetaData.RenderingMetaData.bTransparentPass = true;
                m_sceneNodeMetaData.RenderingMetaData.bOpaquePass = false;
            }else{
                m_sceneNodeMetaData.RenderingMetaData.bTransparentPass = false;
                m_sceneNodeMetaData.RenderingMetaData.bOpaquePass = true;
            }
        }

        for (auto& child : m_children)
        {
            child->Update();
        }
    }

    void SceneNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanStructs::RenderContext* renderingContext) const
    {
        if (m_mesh && m_sceneNodeMetaData.IsVisible)
        {

            // frustrum culling
            if (m_sceneNodeMetaData.FrustumCull && GlobalVariables::RenderingOptions::EnableFrustrumCulling)
            {
                if (!VulkanUtils::IsInViewFrustum(
                        &m_mesh->GetMeshData()->bounds,
                        m_transformation->GetModelMatrix(),
                        renderingContext->view, renderingContext->projection)){return;}
            }

            //=====================================================
            // NORMAL SCENE DATA
            //=====================================================
            VulkanStructs::DrawCallData data;
            data.modelMatrix = m_transformation->GetModelMatrix();
            data.firstIndex = 1;
            data.indexCount = m_mesh->GetMeshIndexCount();
            data.indexCount_BB = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);
            data.material = m_mesh->m_currentMaterial;
            data.meshData = m_mesh->GetMeshData();
            data.renderOutline = m_sceneNodeMetaData.IsSelected;
            data.position = m_transformation->GetPosition();

            //=====================================================
            // BOUNDING VOLUME STUFF
            //=====================================================
            data.bounds = &m_mesh->GetMeshData()->bounds;
            data.isEditorBilboard = false;

            //=====================================================
            // SORT BASED ON THE DEPTH
            //=====================================================
            renderingContext->AddDrawCall(m_sceneNodeMetaData.RenderingMetaData,data);

            if (m_sceneNodeMetaData.IsSelected){

                renderingContext->SelectedGeometryPass.emplace_back(data);
            }


        }
    }
} // ApplicationCore
