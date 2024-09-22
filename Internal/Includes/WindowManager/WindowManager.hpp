//
// Created by wpsimon09 on 22/09/24.
//

#ifndef WINDOWMANAGER_HPP
#define WINDOWMANAGER_HPP

#define GLFW_INCLUDE_VULKAN;
#include <GLFW/glfw3.h>



class WindowManager {
public:
    WindowManager(int windowWidth, int windowHeight);

    void InitWindow();

    const int getWindowWidth() const;
    const int getWindowHeight() const;
    GLFWwindow* GetWindow() const;

    static void GetRequiredExtensions(const char**& extensions, uint32_t& count );

    ~WindowManager();
private:
    GLFWwindow* m_window;
    int m_width, m_height;
};


#endif //WINDOWMANAGER_HPP
