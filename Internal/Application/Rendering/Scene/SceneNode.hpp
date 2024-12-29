//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENENODE_HPP
#define SCENENODE_HPP
#include <memory>
#include <vector>

#include "Application/Rendering/Transformations/Transformations.hpp"


namespace VulkanStructs
{
    struct DrawCallData;
    struct RenderContext;
}

namespace ApplicationCore
{
    class Transformations;
    class Mesh;
    struct DrawContext;

    class SceneNode{
    public:
        explicit SceneNode(std::shared_ptr<Mesh> mesh);
        explicit SceneNode();

        void AddChild(const std::shared_ptr<SceneNode>& child);
        void AddChild(std::shared_ptr<Mesh> child);

        std::string_view GetName() {return m_name;} const
        std::shared_ptr<Mesh>& GetMesh() {return m_mesh;}

        void SetName(const std::string& name) {m_name = name;}

        std::vector<std::reference_wrapper<SceneNode>> GetChildren();
        std::vector<std::shared_ptr<SceneNode>> GetChildren2() {return m_children;};
        void Update() const;
        void Render(VulkanStructs::RenderContext* renderingContext) const;


        bool IsParent() const { return m_parent == nullptr; }

        bool HasMesh() const { return m_hasMesh; }
    public:
        Transformations* m_transformation;
    private:
        Transformations m_localTransformation;
        SceneNode* m_parent;
        std::shared_ptr<Mesh> m_mesh;
        std::string m_name;
        std::vector<std::shared_ptr<SceneNode>> m_children;
        bool m_isParentNode = false;
        bool m_hasMesh = false;
    };

}// ApplicationCore
#endif //SCENENODE_HPP
