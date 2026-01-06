//
// Created by simon on 01/01/2026.
//

#include "ComponentDrawUtils.hpp"

#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/ECS/Components/StaticMeshComponent.hpp"
#include "Application/ECS/Components/TransformComponent.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"

namespace VEditor {
ComponentDrawUtils::ComponentDrawUtils(ECS::ECSCoordinator& ecs)
    : m_ecs(ecs)
    , m_drawFunctions{}
{
    //===============================================================================================
    // Dumb design, but for now it suffice, it will store the functions for each component registered
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::TransformComponent>()] =
        [this](const std::vector<ECS::Entity>& entities) { DrawTransformMultiselect(entities); };
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::MetadataComponent>()] =
        [this](const std::vector<ECS::Entity>& entities) { DrawMetadataComponent(entities); };
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::StaticMeshComponent>()] =
        [this](const std::vector<ECS::Entity>& entities) { DrawStaticMeshComponent(entities); };
}

void ComponentDrawUtils::DrawMultiSelect(ECS::Signature signature, const std::vector<ECS::Entity>& entities)
{
    if(entities.empty())
        return;

    // draw the ui
    Draw(signature, entities);
}


void ComponentDrawUtils::Draw(ECS::Signature signature, const std::vector<ECS::Entity>& entities)
{
    for(size_t componentType = 0; componentType < ECS::MAX_COMPONENTS; ++componentType)
    {
        if(signature.test(componentType))
        {
            auto drawFn = m_drawMultiSelectFunctions.find(componentType);
            if(drawFn != m_drawMultiSelectFunctions.end())
            {
                drawFn->second(entities);
            }
        }
    }
}

void ComponentDrawUtils::DrawTransformMultiselect(const std::vector<ECS::Entity>& entities)
{
    auto  entity = entities[0];
    auto& data   = m_ecs.GetComponentFrom<ECS::TransformComponent>(entity);
    ImGui::SetNextItemOpen(true);
    if(ImGui::TreeNodeEx(ICON_FA_MAP " Transformations"))
    {
        RenderOptions<ECS::TransformComponent>(entity);
        // position
        {
            if(ImGui::Button(ICON_FA_REPLY "##ResetPos"))
            {
                data.position = (glm::vec3(0.0f));
            }
            ImGui::SameLine();
            ImGui::DragFloat3(ICON_FA_ARROWS_TO_DOT " Position", &data.position.x, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
        }
        // Scale
        {
            if(ImGui::Button(ICON_FA_REPLY "##ResetScale"))
            {
                data.scale           = (glm::vec3(1.0f));
                m_uniformScaleScalar = 1.0f;
            }

            if(m_isUniformScaleOn)
            {
                ImGui::SameLine();
                ImGui::DragFloat(ICON_FA_VECTOR_SQUARE " Scale", &m_uniformScaleScalar, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
                data.scale = glm::vec3(m_uniformScaleScalar);
            }
            else
            {
                ImGui::SameLine();
                ImGui::DragFloat3(ICON_FA_VECTOR_SQUARE " Scale", &data.scale.x, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
            }
            ImGui::SameLine();
            ImGui::Checkbox(ICON_FA_LOCK, &m_isUniformScaleOn);
        }
        // rotate
        {
            if(ImGui::Button(ICON_FA_REPLY "##ResetRotation"))
            {
                data.rotationEurel = glm::vec3(0.0f);
            }
            ImGui::SameLine();

            ImGui::DragFloat3(ICON_FA_ARROWS_ROTATE " Rotation", &data.rotationEurel.x, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");

            if(ImGui::BeginItemTooltip())
            {
                auto& quat = data.rotation;
                ImGui::Text("Quat: w: %f,x: %f,y: %f,z: %f", quat.w, quat.x, quat.y, quat.z);
                ImGui::EndTooltip();
            }
        }

        ImGui::TreePop();
    }

    if(entities.size() > 1)
    {
        ApplyToAll<ECS::TransformComponent>(data, entities);
    }
}

void ComponentDrawUtils::DrawMetadataComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNodeEx(ICON_FA_CIRCLE_INFO " Informations"))
    {

        auto& data = m_ecs.GetComponentFrom<ECS::MetadataComponent>(entities[0]);
        ImGui::SeparatorText(data.componentLabel.c_str());
        ImGui::InputText("Entity name", data.entityName, IM_ARRAYSIZE(data.entityName));
        ImGui::InputText("Tag", data.tag.data(), data.tag.size(), ImGuiInputTextFlags_ReadOnly);

        ApplyToAll(data, entities);

        ImGui::TreePop();
    }
}

void ComponentDrawUtils::DrawStaticMeshComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNode(ICON_FA_CUBE " Static mesh component"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::StaticMeshComponent>(entities[0]);
        RenderOptions<ECS::StaticMeshComponent>(entities[0]);

        ImGui::InputText("Mesh name", data.meshName, IM_ARRAYSIZE(data.meshName));

        ApplyToAll(data, entities);

        ImGui::TreePop();
    }
}

}  // namespace VEditor