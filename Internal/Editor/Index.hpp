//
// Created by wpsimon09 on 21/12/24.
//

#ifndef INDEX_HPP
#define INDEX_HPP
#include "UserInterface/IUserInterfaceElement.hpp"


namespace ApplicationCore
{
    class Scene;
}

namespace VEditor
{
    class Index: public IUserInterfaceElement {
    public:
        Index(int width, int height);
        void Init() override;
        void Resize(int newWidth, int newHeight) override;
    };
}


#endif //INDEX_HPP
