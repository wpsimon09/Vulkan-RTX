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
class RenderingSystem;
}

namespace VEditor {

class RenderingOptions : public IUserInterfaceElement
{
  public:
    RenderingOptions(ApplicationCore::ApplicationState& applicationState,
                     ApplicationCore::Scene&            scene,
                     Renderer::RenderingSystem*         renderingSystem);

    void Resize(int newWidth, int newHeight) override;
    void Render() override;
    void Update() override;

  private:
    void                               RenderDrawCallListWidndow(Renderer::RenderingSystem* renderingSystem);
    void                               RenderLightInfoWindow(Renderer::RenderingSystem* renderingSystem);
    ApplicationCore::Scene&            m_scene;
    Renderer::RenderingSystem*         m_renderingSystem;
    bool                               m_openDrawCallListWindow = false;
    bool                               m_openLightInfoLigt      = false;
    ApplicationCore::ApplicationState& m_applicationState;
};

}  // namespace VEditor

#endif  //RENDERINGOPTIONS_HPP
