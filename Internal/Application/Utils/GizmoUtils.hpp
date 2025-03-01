//
// Created by wpsimon09 on 01/03/25.
//

#ifndef GIZMOUTILS_HPP
#define GIZMOUTILS_HPP
#include <memory>
#include <glm/fwd.hpp>


namespace ApplicationCore
{
    class SceneNode;

    void RenderAndUseGizmo(std::shared_ptr<ApplicationCore::SceneNode>& m_selectedSceneNode, glm::mat4& view, glm::mat4& projection);

}

#endif //GIZMOUTILS_HPP
