//
// Created by wpsimon09 on 28/12/24.
//

#ifndef SCENEVIEW_HPP
#define SCENEVIEW_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ApplicationCore
{
    class Scene;
    class SceneNode;
}

namespace VEditor {

class SceneView: public IUserInterfaceElement {
public:
    explicit SceneView(const ApplicationCore::Scene& scene);

    void Resize(int newWidth, int newHeight) override;

    void Render() override;

private:
    void CreateTreeView(std::shared_ptr<ApplicationCore::SceneNode>& sceneNode);
    std::shared_ptr<ApplicationCore::SceneNode> m_selectedSceneNode;
    const ApplicationCore::Scene& m_scene;
};

} // VEditor

#endif //SCENEVIEW_HPP
