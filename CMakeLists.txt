cmake_minimum_required(VERSION 3.28)
project(Vulkan-RTX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_CXX_STANDARD_REQUIRED YES)


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
        Internal/Application/Rendering/Mesh/StaticMesh.cpp
        Internal/Application/Rendering/Mesh/StaticMesh.hpp
        Internal/Vulkan/VulkanCore/Pipeline/VPipelineManager.cpp
        Internal/Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp
        Internal/Vulkan/Global/GlobalVulkanEnums.hpp
        Internal/Vulkan/VulkanCore/RenderPass/VRenderPass.cpp
        Internal/Vulkan/VulkanCore/RenderPass/VRenderPass.hpp
        Internal/Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.cpp
        Internal/Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp
        Internal/Vulkan/VulkanCore/VImage/VImage.cpp
        Internal/Vulkan/VulkanCore/VImage/VImage.hpp
        External/VMA/vk_mem_alloc_implementation.cpp
        Internal/Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.cpp
        Internal/Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp
        Internal/Vulkan/VulkanCore/CommandBuffer/VCommandPool.cpp
        Internal/Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp
        Internal/Vulkan/VulkanCore/Synchronization/VSyncPrimitive.cpp
        Internal/Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp
        Internal/Vulkan/VulkanCore/Buffer/VBuffer.cpp
        Internal/Vulkan/VulkanCore/Buffer/VBuffer.hpp
        Internal/Application/AssetsManger/AssetsManager.cpp
        Internal/Application/AssetsManger/AssetsManager.hpp
        Internal/Application/Rendering/Transformations/Transformations.cpp
        Internal/Application/Rendering/Transformations/Transformations.hpp
        Internal/Application/Rendering/Camera/Camera.cpp
        Internal/Application/Rendering/Camera/Camera.hpp
        Internal/Application/Structs/ApplicationStructs.hpp
        Internal/Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.cpp
        Internal/Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp
        Internal/Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.cpp
        Internal/Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp
        Internal/Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp
        Internal/Vulkan/Utils/VUniformBufferManager/VUniform.cpp
        Internal/Vulkan/Utils/VUniformBufferManager/VUniform.hpp
        Internal/Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.cpp
        Internal/Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp
        External/stb_image/stb_image.cpp
        Internal/Vulkan/VulkanCore/Samplers/VSamplers.hpp
        Internal/Vulkan/VulkanCore/Samplers/VSamplers.cpp
        Internal/Vulkan/Global/VulkanStructs.hpp
        Internal/Application/Rendering/Material/Material.cpp
        Internal/Application/Rendering/Material/Material.hpp
        Internal/Application/Rendering/Scene/Scene.cpp
        Internal/Application/Rendering/Scene/Scene.hpp
        Internal/Application/Rendering/Scene/SceneNode.cpp
        Internal/Application/Rendering/Scene/SceneNode.hpp
        Internal/Application/Rendering/Material/MaterialStructs.hpp
        Internal/Application/GLTFLoader/GltfLoader.cpp
        Internal/Application/GLTFLoader/GltfLoader.hpp
        Internal/Editor/UIContext/UIContext.cpp
        Internal/Editor/UIContext/UIContext.hpp
        Internal/Editor/Views/Index.cpp
        Internal/Editor/Views/Index.hpp
        Internal/Editor/Views/UserInterface/IUserInterfaceElement.cpp
        Internal/Editor/Views/UserInterface/IUserInterfaceElement.hpp
        Internal/Editor/Views/ViewPort/ViewPort.cpp
        Internal/Editor/Views/ViewPort/ViewPort.hpp
        Internal/Vulkan/Renderer/Renderers/SceneRenderer.cpp
        Internal/Vulkan/Renderer/Renderers/SceneRenderer.hpp
        Internal/Vulkan/Renderer/Renderers/UserInterfaceRenderer.cpp
        Internal/Vulkan/Renderer/Renderers/UserInterfaceRenderer.hpp
        Internal/Vulkan/Renderer/Renderers/RenderingSystem.cpp
        Internal/Vulkan/Renderer/Renderers/RenderingSystem.hpp
        Internal/Vulkan/Renderer/RenderTarget/RenderTarget.cpp
        Internal/Vulkan/Renderer/RenderTarget/RenderTarget.hpp
        Internal/Vulkan/Renderer/Renderers/BaseRenderer.cpp
        Internal/Vulkan/Renderer/Renderers/BaseRenderer.hpp
        Internal/Editor/UIContext/ViewPortContext.hpp
        Internal/Editor/Editor.cpp
        Internal/Editor/Editor.hpp
        Internal/Editor/Views/SceneView/SceneView.cpp
        Internal/Editor/Views/SceneView/SceneView.hpp
        Internal/Editor/Views/DetailsPanel/DetailsPanel.cpp
        Internal/Editor/Views/DetailsPanel/DetailsPanel.hpp
        Internal/Vulkan/Renderer/Renderers/DebugRenderer.cpp
        Internal/Vulkan/Renderer/Renderers/DebugRenderer.hpp
        Internal/Editor/Views/RenderingOptions/RenderingOptions.cpp
        Internal/Editor/Views/RenderingOptions/RenderingOptions.hpp
        Internal/Editor/Views/FileExplorer/FileExplorer.cpp
        Internal/Editor/Views/FileExplorer/FileExplorer.hpp
        External/ImGuiFileDialog/ImGuiFileDialog.cpp
        Internal/Editor/Views/MenuBar.cpp
        Internal/Editor/Views/MenuBar.hpp
        Internal/Vulkan/Utils/VMeshDataManager/MeshDataManager.cpp
        Internal/Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp
        Internal/Vulkan/Utils/VFactories.hpp
        Internal/Vulkan/Utils/VFactories.cpp
        Internal/Editor/Views/Console/Console.cpp
        Internal/Editor/Views/Console/Console.hpp
        Internal/Application/IntersectionTests/IntersectionTests.hpp
        Internal/Application/IntersectionTests/IntersectionTests.cpp
        Internal/Application/Utils/ApplicationUtils.hpp
        Internal/Application/Rendering/Scene/DirectionLightNode.cpp
        Internal/Application/Rendering/Scene/DirectionLightNode.hpp
        Internal/Application/Utils/MathUtils.hpp
        Internal/Application/Utils/MathUtils.cpp
        Internal/Application/Structs/LightStructs.hpp
        Internal/Application/Rendering/Scene/PointLightNode.cpp
        Internal/Application/Rendering/Scene/PointLightNode.hpp
        Internal/Application/Rendering/Scene/Base/LightNode.cpp
        Internal/Application/Rendering/Scene/Base/LightNode.hpp
        Internal/Application/Utils/LinearyTransformedCosinesValues.hpp
        Internal/Application/Utils/LinearyTransformedCosinesValues.cpp
        Internal/Application/Rendering/Scene/AreaLightNode.cpp
        Internal/Application/Rendering/Scene/AreaLightNode.hpp
)


# Find Vulkan, GLFW, and GLM
find_package(Vulkan REQUIRED)
find_package(assimp REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

# Add executable
add_executable(${TARGET} ${SOURCES})

#ImGui set up
set(IMGUI_PATH "${CMAKE_SOURCE_DIR}/External/imgui")

file(GLOB IMGUI_SOURCES
        ${IMGUI_PATH}/*.cpp
        ${IMGUI_PATH}/backends/imgui_impl_glfw.h
        ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
        ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_PATH}/backends/imgui_impl_vulkan.h
)

add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH})
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH}/backends)

target_link_libraries(${TARGET} PUBLIC ImGui)


# Include GLM headers
add_subdirectory(${CMAKE_SOURCE_DIR}/External/fastgltf)
target_link_libraries(${TARGET} PRIVATE fastgltf)
target_include_directories(${TARGET} PRIVATE ${GLM_INCLUDE_DIRS})


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/Internal/
        ${CMAKE_SOURCE_DIR}/External/
        ${CMAKE_SOURCE_DIR}/External/IconFontCppHeaders
)

file(COPY ${CMAKE_SOURCE_DIR}/Resources DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/Shaders DESTINATION ${CMAKE_BINARY_DIR})

# Set compile options based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TARGET} PRIVtATE ${CFLAGS_RELEASE})
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${TARGET} PRIVATE ${CFLAGS_DEBUG})
endif()

# Link libraries
target_link_libraries(${TARGET} PRIVATE glfw Vulkan::Vulkan assimp)
