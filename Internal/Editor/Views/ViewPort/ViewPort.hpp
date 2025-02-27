//
// Created by wpsimon09 on 21/12/24.
//

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP
#include <glm/vec2.hpp>

#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

struct ImVec2;
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
        explicit ViewPort(ViewPortContext& viewPortContext, ApplicationCore::Scene& scene, WindowManager& windowManager);

        virtual void Render() override;

        void Resize(int newWidth, int newHeight) override;

    private:
        ApplicationCore::Scene& m_scene;
        WindowManager& m_windowManager;
        ViewPortContext& m_viewPortContext;

        int m_previousWidth;
        int m_previousHeight;
        int m_gizmoRectOriginX;
        int m_gizmoRectOriginY;
        /**
         * Calculates position of mouse inside the view port window
         * @param ImageWidth width of the viewport image
         * @return NDC x and y position of the mouse inside the viewport
         */
        glm::vec2 GetMousePositionInViewPort(ImVec2& ImageWidth);
    };

}



#endif //VIEWPORT_HPP
