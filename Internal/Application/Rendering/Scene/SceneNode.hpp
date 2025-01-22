/*
// Created by wpsimon09 on 26/11/24.
*/

#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include <memory>
#include <vector>

#include "Application/Rendering/Transformations/Transformations.hpp"

//=========================
// Forward Declarations
//=========================
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

    //=========================
    // Static Variables
    //=========================
    static int SceneNodeIDCounter;

    //=========================
    // SceneNode Class
    //=========================
    class SceneNode {
    public:
        //=========================
        // Constructors
        //=========================
        explicit SceneNode(std::shared_ptr<Mesh> mesh);
        explicit SceneNode();

        //=========================
        // Hierarchy Management
        //=========================
        void AddChild(const std::shared_ptr<SceneNode>& child);
        void AddChild(std::shared_ptr<Mesh> child);
        SceneNode* GetParent();

        //=========================
        // Visibility Management
        //=========================
        void Setvisibility(bool isVisible);
        bool GetIsVisible() const { return m_isVisible; }

        void SetHiddenInEditor() { m_showInEditor = false; }
        void SetVisibleInEditor() { m_showInEditor = true; }
        bool IsVisibleInEditor() const { return m_showInEditor; }

        //=========================
        // Selection Management
        //=========================
        void Select(bool selectedFromWorld = false);
        void Deselect();
        bool IsSelected() const { return m_isSelected; }
        bool GetisSelected() const { return m_isSelected; }
        bool GetisSelectedFromWorld() const { return m_isSelectedFromWorld; }

        //=========================
        // Transformation
        //=========================
        void Update() const;

        //=========================
        // Rendering
        //=========================
        void Render(VulkanStructs::RenderContext* renderingContext) const;

        //=========================
        // Utility Functions
        //=========================
        bool PreformRayIntersectionTest(Ray& ray);
        bool IsParent() const { return m_parent == nullptr; }
        bool HasMesh() const { return m_hasMesh; }

        //=========================
        // Accessors
        //=========================
        std::string& GetName() { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        std::shared_ptr<Mesh>& GetMesh() { return m_mesh; }

        std::vector<std::reference_wrapper<SceneNode>> GetChildren();
        std::shared_ptr<SceneNode>& GetChild(int index) { return m_children[index]; }
        std::vector<std::shared_ptr<SceneNode>>& GetChildren2() { return m_children; }

        //=========================
        // Operator Overloading
        //=========================
        friend bool operator==(const SceneNode& lhs, const SceneNode& rhs) {
            return lhs.m_ID == rhs.m_ID;
        }

        friend bool operator!=(const SceneNode& lhs, const SceneNode& rhs) {
            return !(lhs == rhs);
        }

    public:
        Transformations* m_transformation;

    private:
        //=========================
        // Member Variables
        //=========================
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
        bool m_isSelectedFromWorld = false;
        bool m_isAnyChildSelected = false;
        int m_ID = 0;
    };

} // namespace ApplicationCore

#endif // SCENENODE_HPP
