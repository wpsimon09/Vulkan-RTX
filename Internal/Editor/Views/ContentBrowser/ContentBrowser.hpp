//
// Created by wpsimon09 on 03/03/25.
//

#ifndef CONTENTBROWSER_HPP
#define CONTENTBROWSER_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"


namespace ApplicationCore {
class AssetsManager;
class Scene;
}  // namespace ApplicationCore

class ContentBrowser : public VEditor::IUserInterfaceElement
{
  public:
    explicit ContentBrowser(ApplicationCore::AssetsManager& assetManager, ApplicationCore::Scene& scene);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    void                            RenderModels();
    void                            RenderMeshes();
    void                            RenderEffects();
    void                            RenderHDRs();
    ApplicationCore::AssetsManager& m_assetManager;
    ApplicationCore::Scene&         m_scene;
    std::string                     m_selectedAsset;
};


#endif  //CONTENTBROWSER_HPP
