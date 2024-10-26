//
// Created by wpsimon09 on 22/09/24.
//

#include "WindowManager.hpp"
#include <GLFW/glfw3.h>
#include <bits/ios_base.h>

#include "Application/Structs/ApplicationStructs.hpp"

WindowManager::WindowManager(int windowWidth, int windowHeight)
{
    m_width = windowWidth;
    m_height = windowHeight;
    m_cameraMovement = { 0.0f, 0.0f, 0.0f };
}

void WindowManager::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan-RTX", NULL, NULL);

    //Call backs
    glfwSetCursorPosCallback(m_window, MousePositionCallback);
    glfwSetMouseButtonCallback(m_window, MouseClickCallback);
    glfwSetScrollCallback(m_window, MouseScrollCallback);
}

int WindowManager::GetWindowWidth() {
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    return m_width;
}

int WindowManager::GetWindowHeight() {
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
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

void WindowManager::MousePositionCallback(GLFWwindow *window, double xpos, double ypos) {
    m_mousePos.x = static_cast<float>(xpos);
    m_mousePos.y = static_cast<float>(ypos);

    auto pointerX = (float)xpos;
    auto pointerY = (float)ypos;
    if (m_isFirstMouse)
    {
        m_lastX = xpos;
        m_lastY = ypos;
        m_isFirstMouse = false;;
    }

    float xOffset = xpos - m_lastX;
    float yOffset = m_lastY - ypos; // Invert the sign here

    m_lastX = xpos;
    m_lastY = ypos;

    xOffset *= 0.01;
    yOffset *= 0.01;

    if (xOffset != 0.0 && m_isMousePressed)
    {
        m_cameraMovement.RotateAzimuthValue =  xOffset;
    }

    if (yOffset != 0.0 && m_isMousePressed)
    {
        m_cameraMovement.RotatePolarValue = -yOffset;
    }
}

void WindowManager::MouseClickCallback(GLFWwindow *window, int button, int action, int mods) {

}

void WindowManager::MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
}
