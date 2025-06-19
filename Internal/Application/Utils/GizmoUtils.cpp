//
// Created by wpsimon09 on 01/03/25.
//

#include "GizmoUtils.hpp"


#include <glm/gtc/type_ptr.hpp>
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "ImGuizmo/ImGuizmo.h"

void ApplicationCore::RenderAndUseGizmo(std::shared_ptr<ApplicationCore::SceneNode>& m_selectedSceneNode, glm::mat4& view, glm::mat4& projection)
{

    if(m_selectedSceneNode)
    {
        ImGuizmo::Enable(true);

        glm::mat4 proj = projection;
        proj[1][1] *= -1;

        glm::mat4 model = m_selectedSceneNode->m_transformation->GetModelMatrix();

        float snap;
        switch(ImGuizmo::currentOperation)
        {
            case ImGuizmo::ROTATE:
                snap = ImGuizmo::SNAP_ROTATE;
                break;
            case ImGuizmo::TRANSLATE:
                snap = ImGuizmo::SNAP_TRANSLATE;
                break;
            case ImGuizmo::SCALE:
                snap = ImGuizmo::SNAP_SNAP_SCALE;
                break;
        }


        ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), ImGuizmo::currentOperation, ImGuizmo::CURRENT_MODE,
                             glm::value_ptr(model), 0, &snap);

        if(model != m_selectedSceneNode->m_transformation->GetModelMatrix())
        {
            glm::mat4 parentTransform = m_selectedSceneNode->GetParent()
                                        ? m_selectedSceneNode->GetParent()->m_transformation->GetModelMatrix()
                                        : glm::mat4(1.0f);

            glm::mat4 newModel = glm::inverse(parentTransform) * model;


            glm::vec3 t, r, s;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(newModel), glm::value_ptr(t), glm::value_ptr(r), glm::value_ptr(s));

            if(ImGuizmo::currentOperation == ImGuizmo::OPERATION::TRANSLATE)
            {
                m_selectedSceneNode->m_transformation->SetPosition(t[0], t[1], t[2]);
            }
            if(ImGuizmo::currentOperation == ImGuizmo::OPERATION::ROTATE)
            {

                glm::vec3 currentRotation(r[0], r[1], r[2]);
                m_selectedSceneNode->m_transformation->SetRotations(currentRotation);
            }
            if(ImGuizmo::currentOperation == ImGuizmo::OPERATION::SCALE)
            {
                m_selectedSceneNode->m_transformation->SetScale(s[0], s[1], s[2]);
            }
        }
    }
    else
    {
        ImGuizmo::Enable(false);
    }
}
