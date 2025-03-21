//
// Created by wpsimon09 on 29/12/24.
//

#include "DetailsPanel.hpp"
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include <imgui.h>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/AreaLightNode.hpp"
#include "Application/Rendering/Scene/DirectionLightNode.hpp"
#include "Application/Rendering/Scene/PointLightNode.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"

namespace VEditor {
    DetailsPanel::DetailsPanel(const ApplicationCore::AssetsManager& assetsManager): m_assetsManager(assetsManager)
    {

    }

    void DetailsPanel::Render()
    {
        ImGui::Begin(ICON_FA_PEN_FANCY " Details");
            if (!m_selectedSceneNode)
            {
                ImGui::TextColored(ImVec4(0.9, 0.2,0.2,1.0), "No scene node selected");
            }else
            {
                RenderNodeStats();
                RenderTransformationsPanel();
                if (m_selectedSceneNode->HasMesh())
                {
                    RenderMaterialEditorPanel();
                    RenderSceneNodeMetaDataToggles();
                }
                if (m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::DirectionalLightNode)
                {
                    RenderDirectionLightUI();
                }

                if (m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::PointLightNode)
                {
                    RenderPointLightUI();
                }

                if (m_selectedSceneNode->GetSceneNodeMetaData().nodeType == ENodeType::AreaLightNode)
                {
                    RenderAreaLightUI();
                }
            }

        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void DetailsPanel::Resize(int newWidth, int newHeight)
    {

    }

    void DetailsPanel::RenderTransformationsPanel()
    {
            ImGui::SetNextItemOpen(true);
           if (ImGui::TreeNodeEx(ICON_FA_MAP " Transformations"))
           {
               // position
               {
                   if (ImGui::Button(ICON_FA_REPEAT"##ResetPos"))
                   {
                       m_selectedSceneNode->m_transformation->SetPosition(glm::vec3(0.0f));
                   }
                   ImGui::SameLine();
                   ImGui::DragFloat3(ICON_FA_ARROWS_TO_DOT " Position", &m_selectedSceneNode->m_transformation->GetPosition().x, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
               }
               // Scale
               {

                   if (ImGui::Button(ICON_FA_REPEAT"##ResetScale"))
                   {
                       m_selectedSceneNode->m_transformation->SetScale(glm::vec3(1.0f));
                       m_uniformScaleScalar = 1.0f;
                   }

                   if (m_isUniformScaleOn)
                   {
                       ImGui::SameLine();
                       ImGui::DragFloat(ICON_FA_VECTOR_SQUARE " Scale", &m_uniformScaleScalar, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
                       m_selectedSceneNode->m_transformation->SetScale(m_uniformScaleScalar);
                   }else
                   {
                       ImGui::SameLine();
                       ImGui::DragFloat3(ICON_FA_VECTOR_SQUARE " Scale", &m_selectedSceneNode->m_transformation->GetScale().x, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");
                   }
                   ImGui::SameLine();
                   ImGui::Checkbox(ICON_FA_LOCK,&m_isUniformScaleOn);
               }
               // rotate
               {

                   if (ImGui::Button(ICON_FA_REPEAT"##ResetRotation"))
                   {
                       m_selectedSceneNode->m_transformation->SetRotations(glm::vec3(0.0f));
                   }
                   ImGui::SameLine();
                   ImGui::DragFloat3(ICON_FA_ARROWS_ROTATE  " Rotation", &m_selectedSceneNode->m_transformation->GetRotations().x, 0.5f, -FLT_MAX, +FLT_MAX, "%.3f");

               }

               ImGui::TreePop();
           }

    }

    void DetailsPanel::RenderMaterialEditorPanel()
    {
        auto meshMaterial = m_selectedSceneNode->GetMesh()->GetMaterial();


        if (ImGui::TreeNode(ICON_FA_PAINTBRUSH" Material"))
        {
            if (ImGui::BeginCombo("Change material", meshMaterial->GetMaterialName().c_str()))
            {
                for (auto& mat : m_assetsManager.GetAllMaterials())
                {
                    std::string lable = mat->IsTransparent() ? ICON_FA_GLASS_WATER : ICON_FA_BRUSH "  " + mat->GetMaterialName();
                    if (ImGui::Selectable(lable.c_str())){
                        m_selectedSceneNode->GetMesh()->SetMaterial(mat);
                    }
                }
                ImGui::EndCombo();

            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_REPEAT"##ResetMaterial"))
            {
                m_selectedSceneNode->GetMesh()->ResetMaterial();
            }

            float colourPickerWidth = 300.0f;
            //==============
            // ALBEDO
            //===============
            ImGui::SeparatorText("Albedo");
            ImGui::Checkbox("Use texture##a",reinterpret_cast<bool*>(&meshMaterial->GetMaterialDescription().features.hasDiffuseTexture));
            ImGui::Checkbox("Is transparent", &meshMaterial->IsTransparent());
            if (meshMaterial->GetMaterialDescription().features.hasDiffuseTexture)
            {

            }else
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(colourPickerWidth);

                if (meshMaterial->IsTransparent()){
                    ImGui::ColorEdit4("Albedo", &meshMaterial->GetMaterialDescription().values.diffuse.x, ImGuiColorEditFlags_NoInputs);
                }
                else{
                    meshMaterial->GetMaterialDescription().values.diffuse.w = 1.0f;
                    ImGui::ColorEdit3("Albedo", &meshMaterial->GetMaterialDescription().values.diffuse.x, ImGuiColorEditFlags_NoInputs);
                }
            }

            //==============
            // ARM
            //===============
            ImGui::SeparatorText("ARM");
            ImGui::Checkbox("Use texture##arm",reinterpret_cast<bool*>(&meshMaterial->GetMaterialDescription().features.hasArmTexture));
            if (meshMaterial->GetMaterialDescription().features.hasArmTexture)
            {

            }else
            {
                ImGui::SeparatorText("Roughness");
                ImGui::SliderFloat("Roughness", &meshMaterial->GetMaterialDescription().values.roughness, 0.0f, 1.0f, "%.3f");

                //==============
                // METALNES
                //===============
                ImGui::SeparatorText("Metalness");
                ImGui::SliderFloat("Metalness", &meshMaterial->GetMaterialDescription().values.metalness, 0.0f, 1.0f, "%.3f");
            }
            //==============
            // EMISSIVE
            //===============
            ImGui::SeparatorText("Emissive");
            ImGui::Checkbox("Use texture##e",reinterpret_cast<bool*>(&meshMaterial->GetMaterialDescription().features.hasEmissiveTexture));
            if (meshMaterial->GetMaterialDescription().features.hasDiffuseTexture)
            {

            }else
            {
                ImGui::SetNextItemWidth(colourPickerWidth);
                ImGui::ColorEdit3("Emission", &meshMaterial->GetMaterialDescription().values.emissive_strength.x, ImGuiColorEditFlags_NoInputs);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode(ICON_FA_WAND_MAGIC_SPARKLES " Effect"))
        {
            ImGui::Text(meshMaterial->GetEffect()->GetName().c_str());

            ImGui::TreePop();
        }

    }

    void DetailsPanel::RenderMeshOnlyUI()
    {
        if (ImGui::TreeNodeEx(ICON_FA_CIRCLE_INFO " Mesh info"))
        {
            auto meshData = m_selectedSceneNode->GetMesh()->GeteMeshInfo();
            ImGui::Text("Triangle count %i",            meshData.numberOfTriangles);
            ImGui::Text("Index count %i",               meshData.indexCount);
            ImGui::Text("Vertex count %i",              meshData.vertexCount);
            ImGui::TreePop();
        }
    }

    void DetailsPanel::RenderDirectionLightUI()
    {
        auto DirLightNode = dynamic_cast<ApplicationCore::DirectionLightNode*>(m_selectedSceneNode.get());

        ImGui::SeparatorText("General light settings");
        ImGui::SetNextItemWidth(600.0f);
        ImGui::ColorEdit3("LightColour", &DirLightNode->GetLightStruct().colour.x, ImGuiColorEditFlags_NoInputs);

        ImGui::DragFloat("Intensity", &DirLightNode->GetLightStruct().colour.w);

    }

    void DetailsPanel::RenderPointLightUI()
    {
        auto pointLightNode = dynamic_cast<ApplicationCore::PointLightNode*>(m_selectedSceneNode.get());

        ImGui::SeparatorText("General light settings");
        ImGui::ColorEdit3("LightColour", &pointLightNode->GetLightStruct().colour.x, ImGuiColorEditFlags_NoInputs);

        ImGui::DragFloat("Intensity", &pointLightNode->GetLightStruct().colour.w);

        ImGui::SliderFloat("Linear", &pointLightNode->GetLightStruct().linearFactor, 0.0014f, 0.7f);
        ImGui::SliderFloat("QuadraticTerm", &pointLightNode->GetLightStruct().quadraticFactor, 0.0001f, 0.01f, "%.3", ImGuiSliderFlags_Logarithmic);


    }

    void DetailsPanel::RenderAreaLightUI()
    {
        auto areaLightNode = dynamic_cast<ApplicationCore::AreaLightNode*>(m_selectedSceneNode.get());

        ImGui::SeparatorText("General light settings");
        ImGui::ColorEdit3("LightColour", &areaLightNode->GetLightStruct().colour.x, ImGuiColorEditFlags_NoInputs);
        ImGui::DragFloat("Intensity", &areaLightNode->GetLightStruct().intensity);
        ImGui::Checkbox("Two sided", &areaLightNode->GetLightStruct().twoSided);

    }

    void DetailsPanel::RenderSceneNodeMetaDataToggles()
    {

        if (ImGui::TreeNode(ICON_FA_SLIDERS " Options"))
        {
            if (m_selectedSceneNode->HasMesh())
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
        if (ImGui::BeginItemTooltip())
        {
            ImGui::SeparatorText("Node data");
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);

            ImGui::TextUnformatted("ID:");
            ImGui::SameLine();
            ImGui::TextUnformatted(std::to_string(m_selectedSceneNode->GetSceneNodeMetaData().ID).c_str());

            ImGui::PopTextWrapPos();
            if (m_selectedSceneNode->HasMesh())
            {
                ImGui::SeparatorText("Mesh data");
                auto& mesh = m_selectedSceneNode->GetMesh();
                ImGui::TextUnformatted("Vertex buffer ID");
                ImGui::SameLine();
                ImGui::TextUnformatted(std::to_string(mesh->GetMeshData()->vertexData.BufferID).c_str());

                ImGui::TextUnformatted("Index buffer ID");
                ImGui::SameLine();
                ImGui::TextUnformatted(std::to_string(mesh->GetMeshData()->indexData.BufferID).c_str());

            }
            ImGui::EndTooltip();
        }
    }
} // VEditor