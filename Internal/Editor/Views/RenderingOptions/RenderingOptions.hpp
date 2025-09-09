//
// Created by wpsimon09 on 30/12/24.
//

#ifndef RENDERINGOPTIONS_HPP
#define RENDERINGOPTIONS_HPP
#include "Application/ApplicationState/ApplicationState.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ApplicationCore {
class Scene;
class ApplicationState;
}  // namespace ApplicationCore
namespace Renderer {
class Frame;
}

namespace VEditor {

class RenderingOptions : public IUserInterfaceElement
{
  public:
    RenderingOptions(ApplicationCore::ApplicationState& applicationState,
                     ApplicationCore::Scene&            scene,
                     Renderer::Frame*         renderingSystem);

    void Resize(int newWidth, int newHeight) override;
    void Render() override;
    void Update() override;

  private:
    void                               RenderDrawCallListWidndow(Renderer::Frame* renderingSystem);
    void                               RenderLightInfoWindow(Renderer::Frame* renderingSystem);
    ApplicationCore::Scene&            m_scene;
    Renderer::Frame*         m_renderingSystem;
    bool                               m_openDrawCallListWindow = false;
    bool                               m_openLightInfoLigt      = false;
    ApplicationCore::ApplicationState& m_applicationState;

    const std::vector<const char*> m_forwardRendererEffects = {
      "Outline",
      "ForwardShader",
      "SkyBox",
      "DebugLine",
      "AlphaMask",
      "AplhaBlend",
      "EditorBilboard",
      "WireFrame",
      "Unknown"
  };
};

}  // namespace VEditor

#endif  //RENDERINGOPTIONS_HPP
