//
// Created by wpsimon09 on 18/10/24.
//

#ifndef VSYNCPRIMITIVE_HPP
#define VSYNCPRIMITIVE_HPP
#include "Vulkan/VulkanCore/VObject.hpp"


namespace VulkanCore {
    class VDevice;

    template<class  T>
    class VSyncPrimitive:public VObject {
    public:
        explicit VSyncPrimitive(const VDevice& device, bool startAtInSignalState = false);
        void Destroy() override;
    private:
        const VDevice& m_device;
        T m_syncPrimitive;
    };


} // VulkanCore

#endif //VSYNCPRIMITIVE_HPP
