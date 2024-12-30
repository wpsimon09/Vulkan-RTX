//
// Created by wpsimon09 on 30/12/24.
//

#ifndef RENDERINGOPTIONS_HPP
#define RENDERINGOPTIONS_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace Renderer
{
    class RenderingSystem;
}

namespace VEditor {

class RenderingOptions: public IUserInterfaceElement {
public:
    RenderingOptions(const Renderer::RenderingSystem* renderingSystem);

    void Resize(int newWidth, int newHeight) override;
    void Render() override;
    void Update() override;
private:
    const Renderer::RenderingSystem* m_renderingSystem;
};

} // VEditor

#endif //RENDERINGOPTIONS_HPP
