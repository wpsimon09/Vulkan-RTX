//
// Created by wpsimon09 on 31/12/24.
//

#ifndef MENUBAR_HPP
#define MENUBAR_HPP
#include "UserInterface/IUserInterfaceElement.hpp"

namespace VEditor {
    class FileExplorer;
    class Editor;

    class MenuBar: public IUserInterfaceElement {
public:
    explicit MenuBar(Editor* editor);
    void Resize(int newWidth, int newHeight) override;
    void Render() override;
    void Update() override;
    private:
        FileExplorer* m_fileExplorer;
        Editor* m_editor;

        bool m_isFileDialoOpen = false;

        void OnImportSelect() ;
    };

} // VEditor

#endif //MENUBAR_HPP
