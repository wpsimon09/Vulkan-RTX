//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENENODE_HPP
#define SCENENODE_HPP
#include <memory>
#include <vector>

namespace ApplicationCore
{
    class Transformations;
    class Mesh;
    struct DrawContext;

    class SceneNode{
    public:
        explicit SceneNode(std::shared_ptr<Mesh> mesh);

        void AddChild(std::shared_ptr<SceneNode> child);

        std::vector<std::shared_ptr<SceneNode>>& GetChildren();

        void Update() const;

        void Render(struct RenderingContext& renderingContext);

    private:
        Transformations& m_transformation;
        std::shared_ptr<SceneNode> m_parent;
        std::shared_ptr<Mesh> m_mesh;
        std::vector<std::shared_ptr<SceneNode>> m_children;
        bool m_isParentNode = false;
    };

}// ApplicationCore
#endif //SCENENODE_HPP
