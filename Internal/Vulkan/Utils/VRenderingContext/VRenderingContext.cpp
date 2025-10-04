//
// Created by wpsimon09 on 23/03/25.
//

#include "VRenderingContext.hpp"

#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include <cstddef>
#include <optional>


void VulkanUtils::RenderContext::GetAllDrawCall(std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& outDrawCalls)
{
    outDrawCalls = drawCalls;
}

std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& VulkanUtils::RenderContext::GetAllDrawCall()
{
    return drawCalls;
}

void VulkanUtils::RenderContext::AddDrawCall(VulkanStructs::VDrawCallData& DrawCall)
{
    drawCalls.emplace_back(GenerateDrawKey(DrawCall), DrawCall);
}

void VulkanUtils::RenderContext::ResetAllDrawCalls()
{
    atmosphereCall = std::nullopt;
    fogDrawCall    = std::nullopt;
    drawCalls.clear();
}

unsigned long VulkanUtils::RenderContext::GenerateDrawKey(VulkanStructs::VDrawCallData& drawCall)
{
    unsigned long key = 0;

    if(drawCall.material)
    {
        key |= (static_cast<unsigned long>(drawCall.material->IsTransparent()) << 63);
        key |= (static_cast<unsigned long>(drawCall.material->GetID() & 0xFFF) << 36);
    }
    key |= (static_cast<unsigned long>(drawCall.effect & 0xFFF) << 47);  // reduce the number of id to be masekd by 1111 1111 1111 = 2pow25 and move it to the 47th position
    key |= (static_cast<unsigned long>(drawCall.vertexData->BufferID & 0xFFF) << 26);
    key |= (static_cast<unsigned long>(drawCall.indexData->BufferID & 0xFFF) << 16);

    drawCall.key = key;

    return key;
}
