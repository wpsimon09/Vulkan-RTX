//
// Created by wpsimon09 on 22/09/24.
//

#include "WindowManager.hpp"
#include <GLFW/glfw3.h>
#include <bits/ios_base.h>

#include "Application/Structs/ApplicationStructs.hpp"

WindowManager::WindowManager(int windowWidth, int windowHeight):    m_cameraMovement{0.0f, 0.0f, 0.0f}, m_clientUpdate()
{
    m_width = windowWidth;
    m_height = windowHeight;
    m_isDirty = true;
    m_isMousePressed = false;
    m_isShiftPressed = false;
}

void WindowManager::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan-RTX", NULL, NULL);
    glfwSetWindowUserPointer(m_window, this);

    //Call backs
    glfwSetFramebufferSizeCallback(m_window, FrameBufferResizeCallback);
    glfwSetCursorPosCallback(m_window, MousePositionCallback);
    glfwSetMouseButtonCallback(m_window, MouseClickCallback);
    glfwSetScrollCallback(m_window, MouseScrollCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
}

int WindowManager::GetWindowWidth() {
    auto winm = reinterpret_cast<WindowManager*>(m_window);
    glfwGetFramebufferSize(m_window, &winm->m_width, &winm->m_height);
    return winm->m_width;
}

int WindowManager::GetWindowHeight() {
    auto winm = reinterpret_cast<WindowManager*>(m_window);
    glfwGetFramebufferSize(m_window, &winm->m_width, &winm->m_height);
    return winm->m_height;
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

    winm->m_cameraMovement.RotateAzimuthValue = 0.0f;
    winm->m_cameraMovement.RotatePolarValue = 0.0f;
    winm->m_cameraMovement.MoveX = 0.0f;
    winm->m_cameraMovement.MoveY = 0.0f;


    // only rotate Azimuth
    if (xOffset != 0.0 && winm->m_isMousePressed && !winm->m_isShiftPressed)
    {
        winm->m_cameraMovement.RotateAzimuthValue =  xOffset;
        winm->m_isDirty = true;
    }

    // only rotate Polar
    if (yOffset != 0.0 && winm->m_isMousePressed && !winm->m_isShiftPressed)
    {
        winm->m_cameraMovement.RotatePolarValue = -yOffset;
        winm->m_isDirty = true;
    }

    // only move X
    if (xOffset != 0.0 && winm->m_isShiftPressed && winm->m_isMousePressed)
    {
        winm->m_cameraMovement.MoveX =  4.3f *  xOffset;
        winm->m_isDirty = true;
    }

    // only move Y
    if (yOffset != 0.0 && winm->m_isShiftPressed && winm->m_isMousePressed)
    {
        winm->m_cameraMovement.MoveY = 4.3f *   yOffset;
        winm->m_isDirty = true;
    }
}

void WindowManager::MouseClickCallback(GLFWwindow *window, int button, int action, int mods) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    GLFWcursor* hand = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CURSOR_NORMAL);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            winm->m_isMousePressed = true;
            glfwSetCursor(winm->m_window, hand);
        }
        else if (action == GLFW_RELEASE)
        {
            winm->m_isMousePressed = false;
            glfwSetCursor(winm->m_window, cursor);
        }
    }

}

void WindowManager::MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer((window)));
    winm->m_cameraMovement.ZoomValue = 4.0f * (float)yoffset;
    winm->m_isDirty = true;
}

void WindowManager::FrameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    winm->m_width = width;
    winm->m_height = height;
    winm->m_cameraMovement.NewHeight = static_cast<float>(height);
    winm->m_cameraMovement.NewWidth = static_cast<float>(width);
}

void WindowManager::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    winm->m_isDirty = true;

    const float movementSpeed = 2.5;

    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
        winm->m_isShiftPressed = true;
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
        winm->m_isShiftPressed = false;
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        winm->m_cameraMovement.ZoomValue = movementSpeed;

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        winm->m_cameraMovement.MoveX = -movementSpeed;;

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        winm->m_cameraMovement.ZoomValue = -movementSpeed;;

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
        winm->m_cameraMovement.MoveX = movementSpeed;

    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        winm->m_clientUpdate.moveLightY += 0.5;
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        winm->m_clientUpdate.moveLightY -= 0.5;
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        winm->m_clientUpdate.moveLightX -= 0.5;
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        winm->m_clientUpdate.moveLightX += 0.5;
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        if (winm->m_clientUpdate.isRTXon)
        {
            winm->m_clientUpdate.isRTXon = false;
        }else
        {
            winm->m_clientUpdate.isRTXon = true;
        }
    }


}
