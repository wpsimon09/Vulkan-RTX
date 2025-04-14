//
// Created by wpsimon09 on 23/03/25.
//

#include "VRenderingContext.hpp"

#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"

bool VulkanUtils::RenderContext::CompareByDeptDesc(const VulkanStructs::DrawCallData& DrawCallA,
                                                   const VulkanStructs::DrawCallData& DrawCallB)
{

    return DrawCallA.depth > DrawCallB.depth;
}

bool VulkanUtils::RenderContext::CompareByDeptAsc(const VulkanStructs::DrawCallData& DrawCallA,
                                                  const VulkanStructs::DrawCallData& DrawCallB)
{
    return DrawCallA.depth < DrawCallB.depth;
}

void VulkanUtils::RenderContext::GetAllDrawCall(std::vector<std::pair<unsigned long, VulkanStructs::DrawCallData>>& outDrawCalls)
{
    outDrawCalls = drawCalls;
}

std::vector<std::pair<unsigned long, VulkanStructs::DrawCallData>>& VulkanUtils::RenderContext::GetAllDrawCall()
{
    return drawCalls;
}

void VulkanUtils::RenderContext::AddDrawCall(VulkanStructs::DrawCallData& DrawCall)
{
    drawCalls.emplace_back(GenerateDrawKey(DrawCall), DrawCall);
}

void VulkanUtils::RenderContext::ResetAllDrawCalls()
{
    drawCalls.clear();
}

unsigned long VulkanUtils::RenderContext::GenerateDrawKey(VulkanStructs::DrawCallData& drawCall)
{
    unsigned long key = 0;

    key |= (static_cast<unsigned long>(drawCall.material->IsTransparent()) << 63);
    key |= (static_cast<unsigned long>(drawCall.effect->GetID() & 0xFFF)
            << 47);  // reduce the number of id to be masekd by 1111 1111 1111 = 2pow25 and move it to the 47th position
    key |= (static_cast<unsigned long>(drawCall.material->GetID() & 0xFFF) << 36);
    key |= (static_cast<unsigned long>(drawCall.vertexData->BufferID & 0xFFF) << 26);
    key |= (static_cast<unsigned long>(drawCall.indexData->BufferID & 0xFFF) << 16);

    drawCall.key = key;

    return key;
}
