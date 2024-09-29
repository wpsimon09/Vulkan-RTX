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

    static int GetWindowWidth() ;
    static int GetWindowHeight();
    GLFWwindow* GetWindow() const;

    static void GetRequiredExtensions(const char**& extensions, uint32_t& count );

    ~WindowManager();
private:
    static inline GLFWwindow* m_window;
    static inline int m_width, m_height;
};


#endif //WINDOWMANAGER_HPP
