//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VINSTANCE_HPP
#define VINSTANCE_HPP

#include <string>
#include <vulkan/vulkan.hpp>

namespace VulkanCore
{
    class VulkanInstance
    {
    public:
        explicit VulkanInstance(std::string appname);
        ~VulkanInstance();
    private:
        vk::Instance m_instance;
    };
}

#endif //VINSTANCE_HPP
