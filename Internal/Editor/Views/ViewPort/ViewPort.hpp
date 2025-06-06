//
// Created by wpsimon09 on 21/12/24.
//

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP
#include <glm/vec2.hpp>

#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

struct ImVec2;
class WindowManager;

namespace ApplicationCore {
class Scene;
}

struct ViewPortContext;

namespace VEditor {
class ViewPort : public IUserInterfaceElement
{
  public:
    explicit ViewPort(ViewPortContext& rasterViewportContext,ViewPortContext& rayTracedViewPortContext, bool &isRayTracing, ApplicationCore::Scene& scene, WindowManager& windowManager);

    virtual void Render() override;

    void Resize(int newWidth, int newHeight) override;

    void SetViewPortContext(ViewPortContext& viewPortContext);

  private:
    void RenderGizmoActions(ImVec2& imageOrigin, ImVec2& imageSize);

    ApplicationCore::Scene& m_scene;
    WindowManager&          m_windowManager;
    ViewPortContext&        m_rasterViewPortContext;
    ViewPortContext&        m_rayTracedViewPortContext;

    float m_previousWidth;
    float m_previousHeight;
    float m_gizmoRectOriginX;
    float m_gizmoRectOriginY;
    bool& m_isRayTracing;

    /**
         * Calculates position of mouse inside the view port window
         * @param ImageWidth width of the viewport image
         * @return NDC x and y position of the mouse inside the viewport
         */
    glm::vec2 GetMousePositionInViewPort(ImVec2& ImageWidth);
};

}  // namespace VEditor


#endif  //VIEWPORT_HPP
