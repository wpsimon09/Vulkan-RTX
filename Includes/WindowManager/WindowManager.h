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
    WindowManager();

    bool InitWindow();

    const float& getWindowWidth() const;
    const float& getWindowHeight() const;
    const glm::vec2& getWindowDimensions();

    ~WindowManager();
private:
    GLFWwindow* m_window;
    float m_width, m_height;


};



#endif //WINDOWMANAGER_H
