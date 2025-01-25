//
// Created by wpsimon09 on 29/12/24.
//

#ifndef DETAILSPANNEL_HPP
#define DETAILSPANNEL_HPP

#include "Application/Rendering/Scene/SceneNode.hpp"
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

    void SetSelectedNode(std::shared_ptr<ApplicationCore::SceneNode> selectedNode)
    {
        m_selectedSceneNode = selectedNode;
        m_uniformScaleScalar = m_selectedSceneNode->m_transformation->GetScale().x;
    };

    bool isSelectedSceneNodeSame(const std::shared_ptr<ApplicationCore::SceneNode>& selectedNode){ return m_selectedSceneNode == selectedNode; };
private:
    bool m_isUniformScaleOn = false;
    float m_uniformScaleScalar = 1.0f;
    std::shared_ptr<ApplicationCore::SceneNode> m_selectedSceneNode = nullptr;

    void RenderTransformationsPanel();
    void RenderMaterialEditorPanel();
    void RenderMeshOnlyUI();
};

} // VEditor

#endif //DETAILSPANNEL_HPP
