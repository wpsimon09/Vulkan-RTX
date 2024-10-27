//
// Created by wpsimon09 on 22/09/24.
//

#include "WindowManager.hpp"
#include <GLFW/glfw3.h>
#include <bits/ios_base.h>

#include "Application/Structs/ApplicationStructs.hpp"

WindowManager::WindowManager(int windowWidth, int windowHeight):m_cameraMovement{0.0f, 0.0f, 0.0f}
{
    m_width = windowWidth;
    m_height = windowHeight;
    m_isDirty = false;
}

void WindowManager::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan-RTX", NULL, NULL);
    glfwSetWindowUserPointer(m_window, this);

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
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(m_window));
    winm->m_mousePos.x = static_cast<float>(xpos);
    winm->m_mousePos.y = static_cast<float>(ypos);

    auto pointerX = (float)xpos;
    auto pointerY = (float)ypos;
    if (winm->m_isFirstMouse)
    {
        winm->m_lastX = xpos;
        winm->m_lastY = ypos;
        winm->m_isFirstMouse = false;;
    }

    float xOffset = xpos - winm->m_lastX;
    float yOffset = winm->m_lastY - ypos; // Invert the sign here

    winm->m_lastX = xpos;
    winm->m_lastY = ypos;

    xOffset *= 0.01;
    yOffset *= 0.01;

    if (xOffset != 0.0 && winm->m_isMousePressed)
    {
        // have isDirty = true here and in the getter set dirty to false so that cmare is not calculating new stuff every frameP
        // in camera update send this to the camera to know how much to rotate, if nothing happend than dont send it
        winm->m_cameraMovement.RotateAzimuthValue =  xOffset;
    }

    if (yOffset != 0.0 && winm->m_isMousePressed)
    {
        winm->m_cameraMovement.RotatePolarValue = -yOffset;
    }
}

void WindowManager::MouseClickCallback(GLFWwindow *window, int button, int action, int mods) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));

}

void WindowManager::MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer((window)));
    winm->m_cameraMovement.ZoomValue = (float)yoffset;
}
