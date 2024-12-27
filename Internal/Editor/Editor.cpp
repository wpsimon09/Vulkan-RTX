//
// Created by wpsimon09 on 27/12/24.
//

#include "Editor.hpp"

#include "Index.hpp"
#include "UIContext/UIContext.hpp"
#include "ViewPort/ViewPort.hpp"

namespace VEditor {
    Editor::Editor(UIContext& uiContext): m_uiContext(uiContext)
    {
        auto index = std::make_unique<VEditor::Index>(uiContext.m_windowManager.GetWindowWidth(), uiContext.m_windowManager.GetWindowHeight());

        auto viewPort = std::make_unique<ViewPort>(uiContext.m_viewports[ViewPortType::eMain]);
        index->m_uiChildren.emplace_back(std::move(viewPort));

        m_uiElements.emplace_back(std::move(index));
    }

    void Editor::Init()
    {
        Utils::Logger::LogInfo("Initialization of visual editor");
        auto start = std::chrono::high_resolution_clock::now();

        for (auto& uiElement : m_uiElements)
        {
            uiElement->Init();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        Utils::Logger::LogInfo("Visual editor initialized in: " + std::to_string(duration.count()) + "ms !");
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