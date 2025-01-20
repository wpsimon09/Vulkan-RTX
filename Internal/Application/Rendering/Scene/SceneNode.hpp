//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENENODE_HPP
#define SCENENODE_HPP
#include <memory>
#include <vector>

#include "Application/Rendering/Transformations/Transformations.hpp"


struct Ray;

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

    static int SceneNodeIDCounter;

    class SceneNode{
    public:
        explicit SceneNode(std::shared_ptr<Mesh> mesh);
        explicit SceneNode();

        void AddChild(const std::shared_ptr<SceneNode>& child);
        void AddChild(std::shared_ptr<Mesh> child);
        void Setvisibility(bool isVisible);

        void SetHiddenInEditor() {m_showInEditor = false;}
        void SetVisibleInEditor() {m_showInEditor = true;}
        bool IsVisibleInEditor() const {return m_showInEditor;}

        void Select();
        void Deselect();
        bool IsSelected() const {return m_isSelected;}
        bool PreformRayIntersectionTest(Ray& ray);

        std::string& GetName() {return m_name;} const
        std::shared_ptr<Mesh>& GetMesh() {return m_mesh;}

        SceneNode* GetParent();

        void SetName(const std::string& name) {m_name = name;}

        std::vector<std::reference_wrapper<SceneNode>> GetChildren();
        std::shared_ptr<SceneNode>& GetChild(int index) {return m_children[index];}
        bool GetIsVisible(){return m_isVisible;}
        bool GetisSelected() {return m_isSelected;};
        std::vector<std::shared_ptr<SceneNode>>& GetChildren2() {return m_children;};
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
        bool m_isVisible = true;
        bool m_showInEditor = true;
        bool m_isSelected = false;
        int m_ID = 0;

    //===========================================================================
    // Operator overloading
    //===========================================================================
    public:
        friend bool operator==(const SceneNode& lhs, const SceneNode& rhs)
        {
            return lhs.m_ID == rhs.m_ID;
        }

        friend bool operator!=(const SceneNode& lhs, const SceneNode& rhs)
        {
            return !(lhs == rhs);
        }
    };

}// ApplicationCore
#endif //SCENENODE_HPP
