//
// Created by wpsimon09 on 29/12/24.
//

#include "DetailsPanel.hpp"
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include <imgui.h>

#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"

namespace VEditor {
    DetailsPanel::DetailsPanel()
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
                RenderTransformationsPanel();
                if (m_selectedSceneNode->HasMesh())
                {
                    RenderMaterialEditorPanel();
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
        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNode(ICON_FA_PAINTBRUSH" Material"))
        {
            auto meshMaterial = m_selectedSceneNode->GetMesh()->GetMaterial();
            float colourPickerWidth = 300.0f;
            //==============
            // ALBEDO
            //===============
            ImGui::SeparatorText("Albedo");
            ImGui::Checkbox("Use texture##a",reinterpret_cast<bool*>(&meshMaterial->GetMaterialDescription().features.hasDiffuseTexture));
            if (meshMaterial->GetMaterialDescription().features.hasDiffuseTexture)
            {

            }else
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(colourPickerWidth);
                ImGui::ColorEdit3("Albedo", &meshMaterial->GetMaterialDescription().values.diffuse.x, ImGuiColorEditFlags_NoInputs);
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
} // VEditor