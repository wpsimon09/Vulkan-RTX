//
// Created by wpsimon09 on 21/12/24.
//

#ifndef USERINTEFACEELEMENT_HPP
#define USERINTEFACEELEMENT_HPP
#include <memory>
#include <vector>

namespace VEditor
{
    struct IUserInterfaceElement {
        virtual void Render();

        std::vector<std::unique_ptr<IUserInterfaceElement>> m_uiChildren;
        virtual ~IUserInterfaceElement() = default;

        virtual void Resize(int newWidth, int newHeight) = 0;

        virtual void Update(){};
    };
}



#endif //USERINTEFACEELEMENT_HPP
