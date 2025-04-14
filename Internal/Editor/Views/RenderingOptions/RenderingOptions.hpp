//
// Created by wpsimon09 on 30/12/24.
//

#ifndef RENDERINGOPTIONS_HPP
#define RENDERINGOPTIONS_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace Renderer {
class RenderingSystem;
}

namespace VEditor {

class RenderingOptions : public IUserInterfaceElement
{
  public:
    RenderingOptions(Renderer::RenderingSystem* renderingSystem);

    void Resize(int newWidth, int newHeight) override;
    void Render() override;
    void Update() override;

  private:
    void                       RenderDrawCallListWidndow(Renderer::RenderingSystem* renderingSystem);
    void                       RenderLightInfoWindow(Renderer::RenderingSystem* renderingSystem);
    Renderer::RenderingSystem* m_renderingSystem;
    bool                       m_openDrawCallListWindow = false;
    bool                       m_openLightInfoLigt      = false;
};

}  // namespace VEditor

#endif  //RENDERINGOPTIONS_HPP
