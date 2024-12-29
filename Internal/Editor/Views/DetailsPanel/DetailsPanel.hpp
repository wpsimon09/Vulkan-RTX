//
// Created by wpsimon09 on 29/12/24.
//

#ifndef DETAILSPANNEL_HPP
#define DETAILSPANNEL_HPP

#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ApplicationCore
{
    class SceneNode;
}

namespace VEditor {

class DetailsPanel : public IUserInterfaceElement {
public:
    DetailsPanel();

    void Render() override;
    void Resize(int newWidth, int newHeight) override;

    void SetSelectedNode(std::shared_ptr<ApplicationCore::SceneNode> selectedNode) {m_selectedSceneNode = selectedNode;};
private:
    std::shared_ptr<ApplicationCore::SceneNode> m_selectedSceneNode = nullptr;
};

} // VEditor

#endif //DETAILSPANNEL_HPP
