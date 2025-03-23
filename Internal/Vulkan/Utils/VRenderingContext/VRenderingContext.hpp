//
// Created by wpsimon09 on 23/03/25.
//

#ifndef VRENDERINGCONTEXT_HPP
#define VRENDERINGCONTEXT_HPP
#include <vector>
#include <glm/mat4x4.hpp>

namespace VulkanStructs
{
    struct DrawCallData;
}

namespace VulkanUtils
{

    struct RenderContext
    {
        glm::mat4 view{};
        glm::mat4 projection{};

        bool RenderBillboards = true;
        bool RenderAABB = false;
        bool WireFrameRendering = false;
        // other flags

        std::vector<std::pair<unsigned short, VulkanStructs::DrawCallData>> drawCalls;

        void ExtractDepthValues(glm::vec3& cameraPosition);

        static bool CompareByDeptDesc(const VulkanStructs::DrawCallData& DrawCallA, const VulkanStructs::DrawCallData& DrawCallB);
        static bool CompareByDeptAsc(const VulkanStructs::DrawCallData& DrawCallA, const VulkanStructs::DrawCallData& DrawCallB);


        void GetAllDrawCall(std::vector<std::pair<unsigned short, VulkanStructs::DrawCallData>>& outDrawCalls);
        std::vector<std::pair<unsigned short, VulkanStructs::DrawCallData>>& GetAllDrawCall();

        void AddDrawCall(VulkanStructs::DrawCallData& DrawCall);

        void ResetAllDrawCalls();


        static unsigned long GenerateDrawKey(VulkanStructs::DrawCallData& drawCall);

    };
}



#endif //VRENDERINGCONTEXT_HPP
