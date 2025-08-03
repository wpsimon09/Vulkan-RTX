#include "RenderingUtils.hpp"

namespace Renderer {
void ConfigureViewPort(const vk::CommandBuffer& cmdB, float w, float h)
{
    //============================================
    // CONFIGURE VIEW PORT
    //===============================================
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;

    viewport.width  = static_cast<float>(w);
    viewport.height = static_cast<float>(h);

    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdB.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{};
    scissors.offset.x      = 0;
    scissors.offset.y      = 0;
    scissors.extent.width  = w;
    scissors.extent.height = h;

    cmdB.setScissor(0, 1, &scissors);
}
}  // namespace Renderer