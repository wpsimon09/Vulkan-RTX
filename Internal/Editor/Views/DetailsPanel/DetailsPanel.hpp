//
// Created by wpsimon09 on 29/12/24.
//

#ifndef DETAILSPANNEL_HPP
#define DETAILSPANNEL_HPP

#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ApplicationCore
{
    class SkyBoxMaterial;
}

namespace ApplicationCore
{
    class AssetsManager;
}

namespace ApplicationCore
{
    class SceneNode;
}

namespace VEditor {

class DetailsPanel : public IUserInterfaceElement {
public:
    DetailsPanel(const ApplicationCore::AssetsManager& assetsManager);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;

    void SetSelectedNode(std::shared_ptr<ApplicationCore::SceneNode> selectedNode)
    {
        m_selectedSceneNode = selectedNode;
        if (m_selectedSceneNode)
        {
            m_uniformScaleScalar = m_selectedSceneNode->m_transformation->GetScale().x;
        }
    };

    bool isSelectedSceneNodeSame(const std::shared_ptr<ApplicationCore::SceneNode>& selectedNode){ return m_selectedSceneNode == selectedNode; };
private:
    bool m_isUniformScaleOn = false;
    float m_uniformScaleScalar = 1.0f;
    std::shared_ptr<ApplicationCore::SceneNode> m_selectedSceneNode = nullptr;
    const ApplicationCore::AssetsManager& m_assetsManager;

    void RenderTransformationsPanel();
    void RenderMaterialEditorPanel();
    void RenderMeshOnlyUI();
    void RenderDirectionLightUI();
    void RenderPointLightUI();
    void RenderAreaLightUI();
    void RenderEnvLightUI();
    void RenderSceneNodeMetaDataToggles();
    void RenderNodeStats();
    void RenderPBRMaterialDetails(ApplicationCore::PBRMaterial* material);
    void RenderSkyBoxMaterialDetails(ApplicationCore::SkyBoxMaterial* material);
};

} // VEditor

#endif //DETAILSPANNEL_HPP
