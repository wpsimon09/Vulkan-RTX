//
// Created by wpsimon09 on 27/12/24.
//

#ifndef EDITOR_HPP
#define EDITOR_HPP
#include <memory>
#include <vector>

namespace VEditor {

class Index;
class UIContext;
class IUserInterfaceElement;

class Editor {
public:
    Editor(UIContext& uiContext);

    void Render();
    void Update();
    void Resize(int newWidth, int newHeight);
private:
    UIContext& m_uiContext;

    std::vector<std::unique_ptr<VEditor::IUserInterfaceElement>> m_uiElements;

};

} // VEditor

#endif //EDITOR_HPP
