cmake_minimum_required(VERSION 3.28)
project(Vulkan-RTX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_compile_options(-fmodules-ts)

set(CFLAGS_RELEASE "-O2")
set(CFLAGS_DEBUG "-g")

set(TARGET Vulkan-RTX)
set(SOURCES
        src/main.cpp
        Internal/VulkanRtx.cpp
        Internal/VulkanRtx.hpp
        Internal/Application/WindowManager/WindowManager.cpp
        Internal/Application/WindowManager/WindowManager.hpp
        Internal/Vulkan/Global/GlobalState.hpp
        Internal/Application/Logger/Logger.cpp
        Internal/Application/Logger/Logger.hpp
        Internal/Vulkan/VulkanCore/Instance/VInstance.cpp
        Internal/Vulkan/VulkanCore/Instance/VInstance.hpp
        Internal/Vulkan/Utils/VChecker.cpp
        Internal/Vulkan/Utils/VChecker.hpp
        Internal/Vulkan/Global/GlobalVariables.hpp
        Internal/Vulkan/VulkanCore/Device/VDevice.cpp
        Internal/Vulkan/VulkanCore/Device/VDevice.hpp
        Internal/Vulkan/Global/GlobalStructs.hpp
        Internal/Vulkan/Utils/VGeneralUtils.hpp
        Internal/Vulkan/Utils/VGeneralUtils.cpp
        Internal/Vulkan/VulkanCore/SwapChain/VSwapChain.cpp
        Internal/Vulkan/VulkanCore/SwapChain/VSwapChain.hpp
        Internal/Vulkan/VulkanCore/Shader/VShader.cpp
        Internal/Vulkan/VulkanCore/Shader/VShader.hpp
        Internal/Vulkan/VulkanCore/VObject.cpp
        Internal/Vulkan/VulkanCore/VObject.hpp
        Internal/Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.cpp
        Internal/Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp
        Internal/Application/VertexArray/VertexArray.cpp
        Internal/Application/VertexArray/VertexArray.hpp
        Internal/Application/Rendering/Mesh/MeshData.hpp
        Internal/Application/Enums/ClientEnums.hpp
        Internal/Application/Client.cpp
        Internal/Application/Client.hpp
        Internal/Application/Rendering/Mesh/Mesh.cpp
        Internal/Application/Rendering/Mesh/Mesh.hpp
        Internal/Vulkan/VulkanCore/Pipeline/VPipelineManager.cpp
        Internal/Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp
        Internal/Vulkan/Global/GlobalVulkanEnums.hpp
        Internal/Vulkan/VulkanCore/RenderPass/VRenderPass.cpp
        Internal/Vulkan/VulkanCore/RenderPass/VRenderPass.hpp
        Internal/Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.cpp
        Internal/Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp
        Internal/Vulkan/Utils/VImage/VImage.cpp
        Internal/Vulkan/Utils/VImage/VImage.hpp
        External/VMA/vk_mem_alloc_implementation.cpp
        Internal/Vulkan/VulkanCore/CommandBuffer/CommandBuffer.cpp
        Internal/Vulkan/VulkanCore/CommandBuffer/CommandBuffer.hpp
        Internal/Vulkan/VulkanCore/CommandBuffer/CommandPool.cpp
        Internal/Vulkan/VulkanCore/CommandBuffer/CommandPool.hpp

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
        ${CMAKE_SOURCE_DIR}/Internal/
)

# Set compile options based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TARGET} PRIVtATE ${CFLAGS_RELEASE})
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${TARGET} PRIVATE ${CFLAGS_DEBUG})
endif()

# Link libraries
target_link_libraries(${TARGET} PRIVATE glfw Vulkan::Vulkan assimp)