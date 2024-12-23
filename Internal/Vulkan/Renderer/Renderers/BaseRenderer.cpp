//
// Created by wpsimon09 on 23/12/24.
//

#include "BaseRenderer.hpp"

namespace Renderer {
    BaseRenderer::BaseRenderer(const VulkanCore::VDevice& device):m_device(device), m_rendererFinishedSemaphore(device, false){
    }
} // Renderer