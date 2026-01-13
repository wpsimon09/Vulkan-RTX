//
// Created by simon on 01/01/2026.
//

#include "ComponentDrawUtils.hpp"

#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/AtmosphereComponent.hpp"
#include "Application/ECS/Components/FogComponent.hpp"
#include "Application/ECS/Components/LightComponents.hpp"
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
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::FogComponent>()] = [this](const std::vector<ECS::Entity>& entities) {
        DrawFogComponent(entities);
    };
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::AtmosphereComponent>()] =
        [this](const std::vector<ECS::Entity>& entities) { DrawAtmosphereComponent(entities); };
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::DirectionalLightComponent>()] =
        [this](const std::vector<ECS::Entity>& entities) { DrawDirectionalLightComponent(entities); };
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::PointLightComponent>()] =
        [this](const std::vector<ECS::Entity>& entities) { DrawPointLightComponent(entities); };
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::AreaLightComponent>()] =
        [this](const std::vector<ECS::Entity>& entities) { DrawAreaLightComponent(entities); };
    m_drawMultiSelectFunctions[ecs.GetComopnentType<ECS::EnvironemntLightComponent>()] =
        [this](const std::vector<ECS::Entity>& entities) { DrawEnvironemtnLightComponent(entities); };
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
        if(RenderOptions<ECS::TransformComponent>(entities) == 1)
        {
            ImGui::TreePop();
            return;
        };
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

    ApplyToAll<ECS::TransformComponent>(data, entities);
}

void ComponentDrawUtils::DrawMetadataComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNodeEx(ICON_FA_CIRCLE_INFO " Informations"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::MetadataComponent>(entities[0]);
        ImGui::SeparatorText(data.componentLabel.c_str());
        ImGui::InputText("Entity name", data.entityName, IM_ARRAYSIZE(data.entityName));
        ImGui::InputText("Tag", data.tag.data(), data.tag.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::Text("UUID: %s", data.uuid.c_str());
        ApplyToAll<ECS::MetadataComponent>(data, entities);

        ImGui::TreePop();
    }
}

void ComponentDrawUtils::DrawStaticMeshComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNode(ICON_FA_CUBE " Static mesh component"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::StaticMeshComponent>(entities[0]);
        if(RenderOptions<ECS::StaticMeshComponent>(entities) == 1)
        {
            ImGui::TreePop();
            return;
        };

        ImGui::InputText("Mesh name", data.meshName, IM_ARRAYSIZE(data.meshName));

        ApplyToAll(data, entities);

        ImGui::TreePop();
    }
}
void ComponentDrawUtils::DrawFogComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNode(ICON_FA_SMOG " Fog component"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::FogComponent>(entities[0]);
        if(RenderOptions<ECS::FogComponent>(entities) == 1)
        {
            ImGui::TreePop();
            return;
        };

        ImGui::ColorEdit3("Fog colour", &data.fogColour.x, ImGuiColorEditFlags_NoInputs);
        ImGui::Checkbox("Ray marched fog", reinterpret_cast<bool*>(&data.rayMarched));

        ImGui::SetItemTooltip("Density of the volume");
        ImGui::DragFloat("Density", &data.fogColour.w, 0.2, 0.0);

        ImGui::SetItemTooltip("G term for the phase function");
        ImGui::SliderFloat("Asymmetry", &data.asymmetryFactor, -1.0, 1.0);

        ImGui::SliderFloat("Ray distance", &data.rayDistance, 1.0, 900.0);

        ImGui::DragFloat("Ray steps", &data.raySteps, 1, 0.0);
        ImGui::DragFloat("Height fall off", &data.heightFallOff, 0.001, 0.0);
        ImGui::DragFloat("Fog height", &data.fogHeight, 1.0f, 0.0f);

        if(ImGui::TreeNode("Advanced"))
        {
            ImGui::SliderFloat("Absorption coeficient", &data.sigma_a, 0.00001, 0.001, "%.5f");
            ImGui::SliderFloat("Scattering coeficient", &data.sigma_s, 0.00001, 0.001, "%.5f");
            ImGui::TreePop();
        }

        ApplyToAll<ECS::FogComponent>(data, entities);
        ImGui::TreePop();
    }
}

void ComponentDrawUtils::DrawAtmosphereComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNodeEx(ICON_FA_MOUNTAIN_SUN " Atmosphere component"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::AtmosphereComponent>(entities[0]);
        if(RenderOptions<ECS::AtmosphereComponent>(entities) == 1)
        {
            ImGui::TreePop();
            return;
        };

        const auto& defaultParams = ECS::AtmosphereComponent{};

        ImGui::DragFloat("Ground radius", &data.groundAlbedo.w, 1.0, 6360);
        ImGui::DragFloat("Atmosphere radius", &data.rayleighScattering.w, 1.0, 6460);
        ImGui::DragFloat("Mie Asymmetry", &data.mieExtinction.w, 0.1, -1.0, 1.0);
        ImGui::Checkbox("Use multiple scattering", reinterpret_cast<bool*>(&data.booleans.x));
        ImGui::Checkbox("Draw sun", reinterpret_cast<bool*>(&data.booleans.y));
        ImGui::Checkbox("Affects sun luminance", reinterpret_cast<bool*>(&data.booleans.z));

        if(ImGui::TreeNode("Light interaction parameters"))
        {
            if(ImGui::Button(ICON_FA_REPLY "##resetmieScat"))
            {
                data.mieScattering = defaultParams.mieScattering;
            }
            ImGui::SameLine();
            ImGui::DragFloat3("Mie scattering", &data.mieScattering.x, 0.0000001, 0.0, 0.2, "%.7f");

            if(ImGui::Button(ICON_FA_REPLY "##resetMieExtinc"))
            {
                data.mieExtinction = defaultParams.mieExtinction;
            }
            ImGui::SameLine();
            ImGui::DragFloat3("Mie exctinction", &data.mieExtinction.x, 0.0000001, 0.0, 0.2, "%.7f");

            if(ImGui::Button(ICON_FA_REPLY "##resetmieAbs"))
            {
                data.mieAbsorption = defaultParams.mieAbsorption;
            }
            ImGui::SameLine();
            ImGui::DragFloat3("Mie absorption", &data.mieAbsorption.x, 0.0000001, 0.0, 0.2, "%.7f");

            if(ImGui::Button(ICON_FA_REPLY "##resetmieRayleighScat"))
            {
                data.rayleighScattering = defaultParams.rayleighScattering;
            }
            ImGui::SameLine();
            ImGui::DragFloat3("Rayleigh scattering", &data.rayleighScattering.x, 0.0000001, 0.0, 0.2, "%.7f");

            ImGui::TreePop();
        }

        ApplyToAll<ECS::AtmosphereComponent>(data, entities);
        ImGui::TreePop();
    }
}

void ComponentDrawUtils::DrawDirectionalLightComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNode(ICON_FA_SUN " Directional light"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::DirectionalLightComponent>(entities[0]);
        if(RenderOptions<ECS::DirectionalLightComponent>(entities) == 1)
        {
            ImGui::TreePop();
            return;
        };


        ImGui::SeparatorText("General light settings");
        ImGui::SetNextItemWidth(600.0f);
        ImGui::ColorEdit3("LightColour", &data.colour.x, ImGuiColorEditFlags_NoInputs);
        ImGui::SliderFloat("Sun radius", &data.radius, 0.01f, 0.5f);
        ImGui::DragFloat("Intensity", &data.colour.w);

        ImGui::SeparatorText("Shadow");

        ImGui::SliderInt("Shadow rays", &data.shadowRaysPerPixel, 1, 64);
        ImGui::SliderFloat("Shadow bias", &data.shadowBias, 0.0001f, 10.0f);
        ImGui::Checkbox("Accumulate shadows", reinterpret_cast<bool*>(&data.accumulate));

        ApplyToAll<ECS::DirectionalLightComponent>(data, entities);
        ImGui::TreePop();
    }
}


void ComponentDrawUtils::DrawPointLightComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNode(ICON_FA_LIGHTBULB " Point light"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::PointLightComponent>(entities[0]);

        if(RenderOptions<ECS::PointLightComponent>(entities) == 1)
        {
            ImGui::TreePop();
            return;
        };

        ImGui::SeparatorText("General light settings");
        ImGui::ColorEdit3("LightColour", &data.colour.x, ImGuiColorEditFlags_NoInputs);
        ImGui::DragFloat("Intensity", &data.colour.w);

        ImGui::SliderFloat("Linear", &data.linearFactor, 0.0014f, 0.7f);
        ImGui::SliderFloat("QuadraticTerm", &data.quadraticFactor, 0.0001f, 0.01f, "%.3", ImGuiSliderFlags_Logarithmic);

        ApplyToAll<ECS::PointLightComponent>(data, entities);
        ImGui::TreePop();
    }
}


void ComponentDrawUtils::DrawAreaLightComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNode(ICON_FA_VECTOR_SQUARE " Area light"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::AreaLightComponent>(entities[0]);
        if(RenderOptions<ECS::AreaLightComponent>(entities) == 1)
        {
            ImGui::TreePop();
            return;
        };

        ImGui::SeparatorText("General light settings");
        ImGui::ColorEdit3("LightColour", &data.colour.x, ImGuiColorEditFlags_NoInputs);
        ImGui::DragFloat("Intensity", &data.colour.w);
        ImGui::Checkbox("Two sided", &data.twoSided);

        ApplyToAll<ECS::AreaLightComponent>(data, entities);
        ImGui::TreePop();
    }
}


void ComponentDrawUtils::DrawEnvironemtnLightComponent(const std::vector<ECS::Entity>& entities)
{
    if(ImGui::TreeNode(ICON_FA_CLOUD_SUN " Environment light"))
    {
        auto& data = m_ecs.GetComponentFrom<ECS::EnvironemntLightComponent>(entities[0]);

        if(RenderOptions<ECS::EnvironemntLightComponent>(entities) == 1)
        {
            ImGui::TreePop();
            return;
        };

        ImGui::Checkbox("Show sky box", &data.showBackground);
        ImGui::DragFloat("Ambient strength", &data.ambientIntensity, 0.5f, 0.0f, 100.0f);

        ApplyToAll<ECS::EnvironemntLightComponent>(data, entities);
        ImGui::TreePop();
    }
}

}  // namespace VEditor