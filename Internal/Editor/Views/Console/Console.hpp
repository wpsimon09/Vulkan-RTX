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

};

} // VEditor

#endif //CONSOLE_HPP
