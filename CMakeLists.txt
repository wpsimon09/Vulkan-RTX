cmake_minimum_required(VERSION 3.28)
project(Vulkan-RTX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_compile_options(-fmodules-ts)

set(TARGET Vulkan-RTX)
set(SOURCES
        src/main.cpp
)


set(CFLAGS_RELEASE "-O2")
set(CFLAGS_DEBUG "-g")


add_library(Modules
        Internal/Vulkan/VSelector.ixx)
target_sources(Modules
        PUBLIC
            FILE_SET CXX_MODULES
                FILES
        Internal/Vulkan/VInstance.ixx
                Includes/WindowManager/WindowManager.ixx
                Includes/Application.ixx

)

# Find Vulkan, GLFW, and GLM
find_package(Vulkan REQUIRED)
find_package(assimp REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

# Add executable
add_executable(${TARGET} ${SOURCES})

# Include GLM headers
target_include_directories(${TARGET} PRIVATE ${GLM_INCLUDE_DIRS})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/Includes
        ${CMAKE_SOURCE_DIR}/Internal/Vulkan
)

# Set compile options based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TARGET} PRIVATE ${CFLAGS_RELEASE})
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${TARGET} PRIVATE ${CFLAGS_DEBUG})
endif()



# Link libraries
target_link_libraries(${TARGET} PRIVATE glfw Vulkan::Vulkan assimp Modules)

# Custom target for running the executable
add_custom_target(run
        COMMAND ./${TARGET}
        COMMAND ./Shaders/compile.sh
        DEPENDS ${TARGET}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Custom target for cleaning the build
add_custom_target(reset
        COMMAND ${CMAKE_COMMAND} -E remove ${TARGET}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)



