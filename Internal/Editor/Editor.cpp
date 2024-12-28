//
// Created by wpsimon09 on 27/12/24.
//

#include "Editor.hpp"

#include "Views/Index.hpp"
#include "UIContext/UIContext.hpp"
#include "Views/SceneView/SceneView.hpp"
#include "Views/ViewPort/ViewPort.hpp"

namespace VEditor {
    Editor::Editor(UIContext& uiContext): m_uiContext(uiContext)
    {
        Utils::Logger::LogInfo("Initialization of visual editor");
        auto start = std::chrono::high_resolution_clock::now();

        auto index = std::make_unique<VEditor::Index>(uiContext.m_windowManager.GetWindowWidth(), uiContext.m_windowManager.GetWindowHeight());

        auto viewPort = std::make_unique<ViewPort>(uiContext.m_viewports[ViewPortType::eMain]);
        index->m_uiChildren.emplace_back(std::move(viewPort));

        auto sceneView = std::make_unique<SceneView>(uiContext.m_scene);
        index->m_uiChildren.emplace_back(std::move(sceneView));

        m_uiElements.emplace_back(std::move(index));

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        Utils::Logger::LogInfo("Visual editor initialized in: " + std::to_string(duration.count()) + "ms !");
    }

    void Editor::Render()
    {
        m_uiContext.BeginRender();
        for (auto& uiElement : m_uiElements)
        {
            uiElement->Render();
        }
        m_uiContext.EndRender();

    }

    void Editor::Update()
    {
        if (m_uiContext.m_windowManager.GetHasResizedStatus())
        {
            for (auto& uiElement : m_uiElements)
            {
                uiElement->Resize(m_uiContext.m_windowManager.GetWindowWidth(), m_uiContext.m_windowManager.GetWindowWidth());
            }
        }
    }

} // VEditor