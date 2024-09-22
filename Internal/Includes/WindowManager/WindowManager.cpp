//
// Created by wpsimon09 on 22/09/24.
//

#include "WindowManager.hpp"


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

void WindowManager::GetRequiredExtensions(const char**& extensions, uint32_t& count)
{
    extensions = glfwGetRequiredInstanceExtensions(&count);
}


WindowManager::~WindowManager()
{
    glfwDestroyWindow(m_window);
}
