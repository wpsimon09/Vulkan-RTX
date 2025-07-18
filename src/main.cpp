//
// Created by wpsimon09 on 16/09/24.
//

#include <iostream>
#include "VulkanRtx.hpp"
#include "Application/Logger/Logger.hpp"


int main()
{
    try
    {
        Application application;
        Utils::Logger::LogSuccess("Starting the application...");
        application.Run();
        Utils::Logger::LogInfo("Application is stopping...");
    }
    catch(std::exception& e)
    {
        Utils::Logger::LogError(e.what());
    }
    return 0;
}
