//
// Created by wpsimon09 on 28/12/24.
//

#ifndef SCENEVIEW_HPP
#define SCENEVIEW_HPP
#include <unordered_map>

#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace VEditor
{
    class DetailsPanel;
}

namespace ApplicationCore
{
    class Scene;
    class SceneNode;
}

namespace VEditor {

class SceneView: public IUserInterfaceElement {
public:
    explicit SceneView( ApplicationCore::Scene& scene);

    void Resize(int newWidth, int newHeight) override;

    void Render() override;

private:

     void CreateTreeView(std::shared_ptr<ApplicationCore::SceneNode> sceneNode)  ;
     ApplicationCore::Scene& m_scene;

     DetailsPanel* m_detailsPanale;
};

} // VEditor

#endif //SCENEVIEW_HPP
