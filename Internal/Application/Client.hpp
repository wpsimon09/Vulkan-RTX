//
// Created by wpsimon09 on 04/10/24.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <memory>
#include <vector>


//TODO: this class is going to boot up the client site of the renderer like geometry creation, assets managers and all of that stuff, instance of this will be creatd in VulkaRT

namespace ApplicationCore
{
    class Mesh;
}

class Client {
public:
    Client() = default;
    void Init();
    void Update();
    ~Client();
private:
    std::vector<std::unique_ptr<class ApplicationCore::Mesh>> m_meshes;

};



#endif //CLIENT_HPP
