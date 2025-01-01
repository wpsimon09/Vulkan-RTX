//
// Created by wpsimon09 on 31/12/24.
//

#ifndef FILEEXPLORER_HPP
#define FILEEXPLORER_HPP
#include <filesystem>

#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace VEditor {

class FileExplorer: public IUserInterfaceElement {
public:
    FileExplorer();

    std::filesystem::path* Open();
    std::filesystem::path* GetPath() {return &m_filePath;};
    std::filesystem::path  GetPathCpy() {return m_filePath;};

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;
private:
    std::filesystem::path m_filePath;
};

} // VEditor

#endif //FILEEXPLORER_HPP
