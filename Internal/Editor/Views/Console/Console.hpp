//
// Created by wpsimon09 on 18/01/25.
//

#ifndef CONSOLE_HPP
#define CONSOLE_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace VEditor {

class Console: public IUserInterfaceElement {
public:
    Console();

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
private:
    bool m_scrollToBottom = true;
    unsigned int m_previousNumberOfLogs = 0;
    const int LogLimit = 700; // entries
};

} // VEditor

#endif //CONSOLE_HPP
