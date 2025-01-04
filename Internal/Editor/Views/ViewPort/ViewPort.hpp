//
// Created by wpsimon09 on 21/12/24.
//

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

class WindowManager;

namespace ApplicationCore
{
    class Scene;
}

struct ViewPortContext;

namespace VEditor
{
    class ViewPort: public IUserInterfaceElement {
    public:
        explicit ViewPort(ViewPortContext& viewPortContext, const ApplicationCore::Scene& scene, const WindowManager& windoeManager);

        virtual void Render() override;

        void Resize(int newWidth, int newHeight) override;

    private:
        const ApplicationCore::Scene& m_scene;
        const WindowManager& m_windowManager;
        ViewPortContext& m_viewPortContext;
    };
}



#endif //VIEWPORT_HPP
