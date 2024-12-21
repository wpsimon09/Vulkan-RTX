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
        Index(const ApplicationCore::Scene& scene);
        void Init() override;
    private:
       ApplicationCore::Scene& m_scene;
    };
}


#endif //INDEX_HPP
