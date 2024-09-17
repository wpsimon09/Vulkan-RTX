//
// Created by wpsimon09 on 17/09/24.
//

#include "WindowManager.h"

#include <stdexcept>

WindowManager::WindowManager(int windowWidth, int windowHeight)
{
    m_width = windowWidth;
    m_height = windowHeight;
}

void WindowManager::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan-RTX", NULL, NULL);
}

const int WindowManager::getWindowWidth() const
{
    return m_width;
}

const int WindowManager::getWindowHeight() const
{
    return m_height;
}

GLFWwindow* WindowManager::GetWindow()const
{
    return m_window;
}

WindowManager::~WindowManager()
{
    glfwDestroyWindow(m_window);
}
