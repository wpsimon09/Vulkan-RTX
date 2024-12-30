//
// Created by wpsimon09 on 27/12/24.
//

#ifndef EDITOR_HPP
#define EDITOR_HPP
#include <memory>
#include <vector>
#include <sys/stat.h>
#include <VMA/vk_mem_alloc.h>

namespace VEditor {

class Index;
class UIContext;
class IUserInterfaceElement;

class Editor {
public:
    Editor(UIContext& uiContext);

    void Render();
    void Update();
    void SetVmaStatis(VmaTotalStatistics& stats) {m_vmaStats = &stats;};
    void Resize(int newWidth, int newHeight);

private:
    UIContext& m_uiContext;
    std::vector<std::unique_ptr<VEditor::IUserInterfaceElement>> m_uiElements;
    VmaTotalStatistics* m_vmaStats;

private:
    void RenderPreformanceOverlay() const;

};

} // VEditor

#endif //EDITOR_HPP
