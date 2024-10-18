//
// Created by wpsimon09 on 18/10/24.
//

#ifndef VSYNCPRIMITIVE_HPP
#define VSYNCPRIMITIVE_HPP
#include "Vulkan/VulkanCore/VObject.hpp"


namespace VulkanCore {
    class VDevice;

    template<typename T>
    class VSyncPrimitive:public VObject {
    public:
        VSyncPrimitive(const VDevice& device, bool startAtInSignalState = false);
        void Destroy() override;
    private:
        T m_syncPrimitive;
    };


} // VulkanCore

#endif //VSYNCPRIMITIVE_HPP
