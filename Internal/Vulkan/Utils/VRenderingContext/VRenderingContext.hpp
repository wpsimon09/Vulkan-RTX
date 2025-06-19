//
// Created by wpsimon09 on 23/03/25.
//

#ifndef VRENDERINGCONTEXT_HPP
#define VRENDERINGCONTEXT_HPP
#include <memory>
#include <vector>
#include <glm/mat4x4.hpp>

namespace VulkanCore {
class VImage2;
}

namespace ApplicationCore {
class SkyBoxMaterial;
}

namespace VulkanStructs {
struct VDrawCallData;
}

namespace VulkanUtils {

struct RenderContext
{
    glm::mat4 view{};
    glm::mat4 projection{};

    bool RenderBillboards   = true;
    bool RenderAABB         = false;
    bool WireFrameRendering = false;
    // other flags

    std::shared_ptr<ApplicationCore::SkyBoxMaterial> SkyBox = nullptr;

    std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>> drawCalls;

    void ExtractDepthValues(glm::vec3& cameraPosition);

    static bool CompareByDeptDesc(const VulkanStructs::VDrawCallData& DrawCallA, const VulkanStructs::VDrawCallData& DrawCallB);
    static bool CompareByDeptAsc(const VulkanStructs::VDrawCallData& DrawCallA, const VulkanStructs::VDrawCallData& DrawCallB);

    VulkanCore::VImage2* hdrCubeMap    = nullptr;
    VulkanCore::VImage2* irradianceMap = nullptr;
    VulkanCore::VImage2* prefilterMap  = nullptr;
    VulkanCore::VImage2* brdfMap       = nullptr;
    VulkanCore::VImage2* dummyCubeMap  = nullptr;

    bool hasSceneChanged = false;

    void GetAllDrawCall(std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& outDrawCalls);
    std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& GetAllDrawCall();

    void AddDrawCall(VulkanStructs::VDrawCallData& DrawCall);

    void ResetAllDrawCalls();

    static unsigned long GenerateDrawKey(VulkanStructs::VDrawCallData& drawCall);
};
}  // namespace VulkanUtils


#endif  //VRENDERINGCONTEXT_HPP
