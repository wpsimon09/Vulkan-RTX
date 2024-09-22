//
// Created by wpsimon09 on 16/09/24.
//

#include <iostream>
#include "VulkanRtx.hpp"
#include "Vulkan/Utils/VLogger.hpp"

int main()
{
    Application application;
    try
    {
        Utils::VLogger::LogSuccess("Starting the application...");
        application.Run();
        Utils::VLogger::LogInfo("Application is stopping...");

    }catch(std::exception& e)
    {
        Utils::VLogger::LogError(e.what());
    }
    return 0;
}
