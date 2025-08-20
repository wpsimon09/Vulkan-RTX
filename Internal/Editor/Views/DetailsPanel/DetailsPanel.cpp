//
// Created by wpsimon09 on 29/12/24.
//

#include "DetailsPanel.hpp"
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include <imgui.h>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Material/SkyBoxMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/AreaLightNode.hpp"
#include "Application/Rendering/Scene/DirectionLightNode.hpp"
#include "Application/Rendering/Scene/FogVolumeNode.hpp"
#include "Application/Rendering/Scene/PointLightNode.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Application/Rendering/Scene/SkyBoxNode.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"

namespace VEditor {
DetailsPanel::DetailsPanel(const ApplicationCore::AssetsManager& assetsManager)
    : m_assetsManager(assetsManager)
{
}

void DetailsPanel::Render()
{
    ImGui::Begin(ICON_FA_PEN_FANCY " Details");
    if(!m_selectedSceneNode)
    {
        ImGui::TextColored(ImVec4(0.9, 0.2, 0.2, 1.0), "No scene node selected");
    }
    else
    {
        RenderNodeStats();
        RenderTransformationsPanel();
        if(m_selectedSceneNode->HasMesh())
        {
            RenderMaterialEditorPanel();
            RenderSceneNodeMetaDataToggles();
        }

        if(m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::DirectionalLightNode)
        {
            RenderDirectionLightUI();
        }

        if(m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::PointLightNode)
        {
            RenderPointLightUI();
        }

        if(m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::AreaLightNode)
        {
            RenderAreaLightUI();
        }

        if(m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::SkyBoxNode)
        {
            RenderEnvLightUI();
        }

        if(m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::FogVolume)
        {
            RenderFogVolumeNodeUI();
        }
    }

    ImGui::End();
    IUserInterfaceElement::Render();
}

void DetailsPanel::Resize(int newWidth, int newHeight) {}

void DetailsPanel::RenderTransformationsPanel()
{
    ImGui::SetNextItemOpen(true);
    if(ImGui::TreeNodeEx(ICON_FA_MAP " Transformations"))
    {
        // position
        {
            if(ImGui::Button(ICON_FA_REPLY "##ResetPos"))
            {
                m_selectedSceneNode->m_transformation->SetPosition(glm::vec3(0.0f));
            }
            ImGui::SameLine();
            ImGui::DragFloat3(ICON_FA_ARROWS_TO_DOT " Position",
                              &m_selectedSceneNode->m_transformation->GetPosition().x, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
        }
        // Scale
        {
            if(ImGui::Button(ICON_FA_REPLY "##ResetScale"))
            {
                m_selectedSceneNode->m_transformation->SetScale(glm::vec3(1.0f));
                m_uniformScaleScalar = 1.0f;
            }

            if(m_isUniformScaleOn)
            {
                ImGui::SameLine();
                ImGui::DragFloat(ICON_FA_VECTOR_SQUARE " Scale", &m_uniformScaleScalar, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
                m_selectedSceneNode->m_transformation->SetScale(m_uniformScaleScalar);
            }
            else
            {
                ImGui::SameLine();
                ImGui::DragFloat3(ICON_FA_VECTOR_SQUARE " Scale", &m_selectedSceneNode->m_transformation->GetScale().x,
                                  0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
            }
            ImGui::SameLine();
            ImGui::Checkbox(ICON_FA_LOCK, &m_isUniformScaleOn);
        }
        // rotate
        {
            if(ImGui::Button(ICON_FA_REPLY "##ResetRotation"))
            {
                m_selectedSceneNode->m_transformation->SetRotations(glm::vec3(0.0f));
            }
            ImGui::SameLine();

            ImGui::DragFloat3(ICON_FA_ARROWS_ROTATE " Rotation",
                              &m_selectedSceneNode->m_transformation->GetRotations().x, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");

            if(ImGui::BeginItemTooltip())
            {
                auto& quat = m_selectedSceneNode->m_transformation->GetRotationsQuat();
                ImGui::Text("Quat: w: %f,x: %f,y: %f,z: %f", quat.w, quat.x, quat.y, quat.z);
                ImGui::EndTooltip();
            }
        }

        ImGui::TreePop();
    }
}

void DetailsPanel::RenderMaterialEditorPanel()
{
    auto meshMaterial = m_selectedSceneNode->GetMesh()->GetMaterial().get();
    if(ImGui::TreeNode(ICON_FA_PAINTBRUSH " Material"))
    {
        if(auto pbrMat = dynamic_cast<ApplicationCore::PBRMaterial*>(meshMaterial))
        {

            RenderPBRMaterialDetails(pbrMat);
        }
        else if(auto skyBoxMat = dynamic_cast<ApplicationCore::SkyBoxMaterial*>(meshMaterial))
        {
            ImGui::SeparatorText("HDR");
            RenderSkyBoxMaterialDetails(skyBoxMat);
        }
        else
        {
            ImGui::Text("Node has no material properties !");
        }
        ImGui::TreePop();
    }

    if(ImGui::TreeNode(ICON_FA_WAND_MAGIC_SPARKLES " Effect"))
    {
        if(ImGui::Button(ICON_FA_REPLY " ## reset efect"))
        {
            m_selectedSceneNode->GetMesh()->GetMaterial()->ResetEffect();
        }

        ImGui::SameLine();


        ImGui::TreePop();
    }
}

void DetailsPanel::RenderMeshOnlyUI()
{
    if(ImGui::TreeNodeEx(ICON_FA_CIRCLE_INFO " Mesh info"))
    {
        auto meshData = m_selectedSceneNode->GetMesh()->GeteMeshInfo();
        ImGui::Text("Triangle count %i", meshData.numberOfTriangles);
        ImGui::Text("Index count %i", meshData.indexCount);
        ImGui::Text("Vertex count %i", meshData.vertexCount);
        ImGui::TreePop();
    }
}


void DetailsPanel::RenderDirectionLightUI()
{
    auto DirLightNode = dynamic_cast<ApplicationCore::DirectionLightNode*>(m_selectedSceneNode.get());

    ImGui::SeparatorText("General light settings");
    ImGui::SetNextItemWidth(600.0f);
    ImGui::ColorEdit3("LightColour", &DirLightNode->GetLightStruct().colour.x, ImGuiColorEditFlags_NoInputs);
    ImGui::SliderFloat("Sun radius", &DirLightNode->GetLightStruct().sunRadius, 0.01, 0.5);
    ImGui::DragFloat("Intensity", &DirLightNode->GetLightStruct().colour.w);

    ImGui::SeparatorText("Shadow");

    ImGui::SliderInt("Shadow rays", &DirLightNode->GetLightStruct().shadowRaysPerPixel, 1, 64);
    ImGui::SliderFloat("Shadow bias", &DirLightNode->GetLightStruct().shadowBias, 0.0001f, 10.0);
}

void DetailsPanel::RenderPointLightUI()
{
    auto pointLightNode = dynamic_cast<ApplicationCore::PointLightNode*>(m_selectedSceneNode.get());

    ImGui::SeparatorText("General light settings");
    ImGui::ColorEdit3("LightColour", &pointLightNode->GetLightStruct().colour.x, ImGuiColorEditFlags_NoInputs);

    ImGui::DragFloat("Intensity", &pointLightNode->GetLightStruct().colour.w);

    ImGui::SliderFloat("Linear", &pointLightNode->GetLightStruct().linearFactor, 0.0014f, 0.7f);
    ImGui::SliderFloat("QuadraticTerm", &pointLightNode->GetLightStruct().quadraticFactor, 0.0001f, 0.01f, "%.3",
                       ImGuiSliderFlags_Logarithmic);
}

void DetailsPanel::RenderAreaLightUI()
{
    auto areaLightNode = dynamic_cast<ApplicationCore::AreaLightNode*>(m_selectedSceneNode.get());

    ImGui::SeparatorText("General light settings");
    ImGui::ColorEdit3("LightColour", &areaLightNode->GetLightStruct().colour.x, ImGuiColorEditFlags_NoInputs);
    ImGui::DragFloat("Intensity", &areaLightNode->GetLightStruct().intensity);
    ImGui::Checkbox("Two sided", &areaLightNode->GetLightStruct().twoSided);
}

void DetailsPanel::RenderEnvLightUI()
{
    auto envLIght = dynamic_cast<ApplicationCore::SkyBoxNode*>(m_selectedSceneNode.get());
    ImGui::Checkbox("Show sky box", &envLIght->m_showBackground);
    ImGui::DragFloat("Ambient strength", &envLIght->GetLightStruct().ambientIntensity, 0.5, 0.0f, 100.0f);
}

void DetailsPanel::RenderSceneNodeMetaDataToggles()
{
    if(ImGui::TreeNode(ICON_FA_SLIDERS " Options"))
    {
        if(m_selectedSceneNode->HasMesh())
        {
            ImGui::Checkbox("Frustrum cull", &m_selectedSceneNode->GetSceneNodeMetaData().FrustumCull);
            ImGui::Checkbox("Cast Shadows", &m_selectedSceneNode->GetSceneNodeMetaData().CastsShadows);
        }
        ImGui::TreePop();
    }
}

void DetailsPanel::RenderNodeStats()
{
    ImGui::TextDisabled("(?) Node data");
    if(ImGui::BeginItemTooltip())
    {
        ImGui::SeparatorText("Node data");
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);

        ImGui::TextUnformatted("ID:");
        ImGui::SameLine();
        ImGui::TextUnformatted(std::to_string(m_selectedSceneNode->GetSceneNodeMetaData().ID).c_str());

        ImGui::PopTextWrapPos();
        if(m_selectedSceneNode->HasMesh())
        {
            ImGui::SeparatorText("Mesh data");
            auto& mesh = m_selectedSceneNode->GetMesh();
            ImGui::TextUnformatted("Vertex buffer ID");
            ImGui::SameLine();
            ImGui::TextUnformatted(std::to_string(mesh->GetMeshData()->vertexData.BufferID).c_str());

            ImGui::TextUnformatted("Index buffer ID");
            ImGui::SameLine();
            ImGui::TextUnformatted(std::to_string(mesh->GetMeshData()->indexData.BufferID).c_str());

            ImGui::TextUnformatted("Vertex buffer address");
            ImGui::SameLine();
            ImGui::TextUnformatted(std::to_string(mesh->GetMeshData()->vertexData.bufferAddress).c_str());

            ImGui::TextUnformatted("Index buffer address");
            ImGui::SameLine();
            ImGui::TextUnformatted(std::to_string(mesh->GetMeshData()->indexData.bufferAddress).c_str());
        }
        ImGui::EndTooltip();
    }
}

void DetailsPanel::RenderPBRMaterialDetails(ApplicationCore::PBRMaterial* material)
{
    if(m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::MeshNode)
    {
        if(ImGui::BeginCombo("Change material", material->GetMaterialName().c_str()))
        {
            for(auto& mat : m_assetsManager.GetAllMaterials())
            {
                std::string lable = mat->IsTransparent() ? ICON_FA_GLASS_WATER : ICON_FA_BRUSH "  " + mat->GetMaterialName();
                if(ImGui::Selectable(lable.c_str()))
                {
                    m_selectedSceneNode->GetMesh()->SetMaterial(mat);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if(ImGui::Button(ICON_FA_REPLY "##ResetMaterial"))
        {
            m_selectedSceneNode->GetMesh()->ResetMaterial();
        }

        float colourPickerWidth = 300.0f;
        //==============
        // ALBEDO
        //===============
        ImGui::SeparatorText("Albedo");
        ImGui::Checkbox("Use texture##a", reinterpret_cast<bool*>(&material->GetMaterialDescription().features.hasAlbedoTexture));
        ImGui::Checkbox("Is transparent", &material->IsTransparent());
        if(material->GetMaterialDescription().features.hasAlbedoTexture)
        {
        }
        else
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(colourPickerWidth);

            if(material->IsTransparent())
            {
                ImGui::ColorEdit4("Albedo", &material->GetMaterialDescription().values.albedo.x, ImGuiColorEditFlags_NoInputs);
            }
            else
            {
                material->GetMaterialDescription().values.albedo.w = 1.0f;
                ImGui::ColorEdit3("Albedo", &material->GetMaterialDescription().values.albedo.x, ImGuiColorEditFlags_NoInputs);
            }
        }

        //=================
        // NORMAL texture
        //=================
        ImGui::SeparatorText("Normal");
        ImGui::Checkbox("Use normal texture", reinterpret_cast<bool*>(&material->GetMaterialDescription().features.hasNormalTexture));

        //==============
        // ARM
        //===============
        ImGui::SeparatorText("ARM");
        ImGui::Checkbox("Use texture##arm", reinterpret_cast<bool*>(&material->GetMaterialDescription().features.hasArmTexture));
        if(material->GetMaterialDescription().features.hasArmTexture)
        {
        }
        else
        {
            ImGui::SeparatorText("Roughness");
            ImGui::SliderFloat("Roughness", &material->GetMaterialDescription().values.roughness, 0.0f, 1.0f, "%.3f");

            //==============
            // METALNES
            //===============
            ImGui::SeparatorText("Metalness");
            ImGui::SliderFloat("Metalness", &material->GetMaterialDescription().values.metalness, 0.0f, 1.0f, "%.3f");
        }
        //==============
        // EMISSIVE
        //===============
        ImGui::SeparatorText("Emissive");
        ImGui::Checkbox("Use texture##e", reinterpret_cast<bool*>(&material->GetMaterialDescription().features.hasEmissiveTexture));
        if(material->GetMaterialDescription().features.hasEmissiveTexture)
        {
        }
        else
        {
            ImGui::DragFloat("Emissive strength", &material->GetMaterialDescription().values.emissive_strength.w, 0.5f, 0.0f);
            ImGui::SetNextItemWidth(colourPickerWidth);
            ImGui::ColorEdit3("Emission", &material->GetMaterialDescription().values.emissive_strength.x, ImGuiColorEditFlags_NoInputs);
        }
    }
}

void DetailsPanel::RenderSkyBoxMaterialDetails(ApplicationCore::SkyBoxMaterial* material)
{
    if(ImGui::BeginCombo("Change material", material->GetMaterialName().c_str()))
    {
        for(auto& mat : m_assetsManager.GetAllSkyBoxMaterials())
        {
            std::string lable = ICON_FA_MOUNTAIN "  " + mat->GetMaterialName();
            if(ImGui::Selectable(lable.c_str()))
            {
                m_selectedSceneNode->GetMesh()->SetMaterial(mat);
            }
            ImGui::Text(reinterpret_cast<const char*>(mat->GetHDRTexture()->GetHandle().get()));
        }
        ImGui::EndCombo();
    }
}

void DetailsPanel::RenderFogVolumeNodeUI()
{
    if(auto fogNode = dynamic_cast<ApplicationCore::FogVolumeNode*>(m_selectedSceneNode.get()))
    {
        ImGui::ColorEdit3("Fog colour", &fogNode->GetParameters().fogColour.x, ImGuiColorEditFlags_NoInputs);
        ImGui::Checkbox("Ray marched fog", reinterpret_cast<bool*>(&fogNode->GetParameters().rayMarched));

        ImGui::SetItemTooltip("Density of the volume");
        ImGui::DragFloat("Density", &fogNode->GetParameters().fogColour.w, 0.2, 0.0);

        ImGui::SetItemTooltip("G term for the phase function");
        ImGui::SliderFloat("Asymmetry", &fogNode->GetParameters().asymmetryFactor, -1.0, 1.0);

        ImGui::SliderFloat("Ray distance", &fogNode->GetParameters().rayDistance, 1.0, 900.0);

        ImGui::DragFloat("Ray steps", &fogNode->GetParameters().raySteps, 1, 0.0);
        ImGui::DragFloat("Height fall off", &fogNode->GetParameters().heightFallOff, 0.001, 0.0);
        ImGui::DragFloat("Fog height", &fogNode->GetParameters().fogHeight, 1.0f, 0.0f);

        if(ImGui::TreeNode("Advanced"))
        {
            ImGui::SliderFloat("Absorption coeficient", &fogNode->GetParameters().sigma_a, 0.00001, 0.001, "%.5f");
            ImGui::SliderFloat("Scattering coeficient", &fogNode->GetParameters().sigma_s, 0.00001, 0.001, "%.5f");
            ImGui::TreePop();
        }
    }
}

}  // namespace VEditor
