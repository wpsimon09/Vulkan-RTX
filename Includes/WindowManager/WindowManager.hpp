//
// Created by wpsimon09 on 17/09/24.
//

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#define GLFW_INCLUDE_VULKAN;
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

class WindowManager {
public:
    WindowManager(int windowWidth, int windowHeight);

    void InitWindow();

    const int getWindowWidth() const;
    const int getWindowHeight() const;
    GLFWwindow* GetWindow() const;

    ~WindowManager();
private:
    GLFWwindow* m_window;
    int m_width, m_height;


};



#endif //WINDOWMANAGER_H
