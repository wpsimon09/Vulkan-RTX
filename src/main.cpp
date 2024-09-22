//
// Created by wpsimon09 on 16/09/24.
//

#include <iostream>

import Application;
import VLogger;
int main()
{
    Application application;
    try
    {
        VLogger::LogInfo("Starting the application...");
        application.Run();
        VLogger::LogInfo("Application is stopping...");

    }catch(std::exception& e)
    {
        VLogger::LogError(e.what());
    }
    return 0;
}
