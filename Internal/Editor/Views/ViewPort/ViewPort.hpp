//
// Created by wpsimon09 on 21/12/24.
//

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

struct ViewPortContext;

namespace VEditor
{
    class ViewPort: public IUserInterfaceElement {
    public:
        explicit ViewPort(ViewPortContext& viewPortContext);

        virtual void Render() override;

        void Resize(int newWidth, int newHeight) override;

    private:
        ViewPortContext& m_viewPortContext;
    };
}



#endif //VIEWPORT_HPP
