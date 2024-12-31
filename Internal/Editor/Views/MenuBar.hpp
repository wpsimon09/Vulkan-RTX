//
// Created by wpsimon09 on 31/12/24.
//

#ifndef MENUBAR_HPP
#define MENUBAR_HPP
#include "UserInterface/IUserInterfaceElement.hpp"

namespace VEditor {
    class Editor;

    class MenuBar: public IUserInterfaceElement {
public:
    MenuBar(Editor* editor);
    void Resize(int newWidth, int newHeight) override;
    void Render() override;
    void Update() override;
    private:
        Editor* m_editor;
    };

} // VEditor

#endif //MENUBAR_HPP
