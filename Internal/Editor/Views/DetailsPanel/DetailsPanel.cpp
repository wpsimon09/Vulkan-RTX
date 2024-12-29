//
// Created by wpsimon09 on 29/12/24.
//

#include "DetailsPanel.hpp"
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include <imgui.h>

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
                ImGui::SeparatorText("Transformations");
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

            }

        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void DetailsPanel::Resize(int newWidth, int newHeight)
    {

    }
} // VEditor