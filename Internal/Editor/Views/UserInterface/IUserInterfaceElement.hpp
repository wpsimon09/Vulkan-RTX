//
// Created by wpsimon09 on 21/12/24.
//

#ifndef USERINTEFACEELEMENT_HPP
#define USERINTEFACEELEMENT_HPP
#include <memory>
#include <vector>

namespace VEditor
{
    class UIContext;
}

namespace VEditor
{
    struct IUserInterfaceElement {

        std::vector<std::unique_ptr<IUserInterfaceElement>> m_uiChildren;

        virtual void Render();

        virtual void Resize(int newWidth, int newHeight) = 0;

        virtual void Update(){};

        virtual ~IUserInterfaceElement() = default;

        bool m_isOpen = true;

    };
}



#endif //USERINTEFACEELEMENT_HPP
