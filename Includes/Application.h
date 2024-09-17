//
// Created by wpsimon09 on 17/09/24.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include <memory>

#include "WindowManager/WindowManager.h"


class Application {
public:
    Application() = default;

    void Init();

    void Run();

    void Update();

    void Render();

    void MainLoop();

    ~Application() = default;

private:
    std::unique_ptr<WindowManager> m_windowManager;

};



#endif //APPLICATION_H
