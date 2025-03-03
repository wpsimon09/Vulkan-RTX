/*
// Created by wpsimon09 on 26/11/24.
*/

#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include <memory>
#include <vector>

#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/Structs/ApplicationStructs.hpp"

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
    class StaticMesh;
    struct DrawContext;

    //=========================
    // Static Variables
    //=========================
    static int SceneNodeIDCounter;

    //=========================
    // SceneNode Class
    //=========================
    class SceneNode:public std::enable_shared_from_this<SceneNode>
    {
    public:
        //=========================
        // Constructors
        //=========================
        explicit SceneNode(std::shared_ptr<StaticMesh> mesh);
        explicit SceneNode();

        //=========================
        // Hierarchy Management
        //=========================
        void AddChild(const std::shared_ptr<SceneNode>& child);
        void AddChild(std::shared_ptr<StaticMesh> child);
        SceneNode* GetParent();

        //=========================
        // Visibility Management
        //=========================
        void Setvisibility(bool isVisible);
        bool GetIsVisible() const { return m_sceneNodeMetaData.IsVisible; }

        void SetHiddenInEditor() { m_sceneNodeMetaData.ShowInEditor = false; }
        void SetVisibleInEditor() { m_sceneNodeMetaData.ShowInEditor = true; }
        bool IsVisibleInEditor() const { return m_sceneNodeMetaData.ShowInEditor; }


        //=========================
        // Selection Management
        //=========================
        void SetExpansionState(bool expansionState) {m_sceneNodeMetaData.IsOpen = expansionState;}
        void Select(bool selectedFromWorld = false);
        void Deselect();
        bool IsSelected() const { return m_sceneNodeMetaData.IsSelected; }
        bool IsOpen() const {return m_sceneNodeMetaData.IsOpen;}
        SceneNodeMetaData& GetSceneNodeMetaData() { return m_sceneNodeMetaData; }

        //=========================
        // Transformation
        //=========================
        virtual void Update();

        //=========================
        // Rendering
        //=========================
        virtual void Render(VulkanStructs::RenderContext* renderingContext) const;

        //=========================
        // Utility Functions
        //=========================
        void PreformRayIntersectionTest(Ray& ray, std::vector<std::shared_ptr<SceneNode>>& result);
        void SetLocalTransform(Transformations& transformations) { m_localTransformation = transformations; m_transformation = &m_localTransformation; };
        bool IsParent() const { return m_parent == nullptr; }
        bool HasMesh() const { return m_sceneNodeMetaData.HasMesh; }
        float GetDistanceFromCamera(glm::vec3 cameraPosition);

        //=========================
        // Accessors
        //=========================
        std::string& GetName() { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        std::shared_ptr<StaticMesh>& GetMesh() { return m_mesh; }

        std::vector<std::reference_wrapper<SceneNode>> GetChildrenByWrapper();
        std::shared_ptr<SceneNode>& GetChild(int index) { return m_children[index]; }
        std::vector<std::shared_ptr<SceneNode>>& GetChildrenByRef() { return m_children; }

        //=========================
        // Callback functions
        //=========================
        virtual void ProcessNodeRemove(){ };

        //=========================
        // Operator Overloading
        //=========================
        friend bool operator==(const SceneNode& lhs, const SceneNode& rhs) {
            return lhs.m_sceneNodeMetaData.ID == rhs.m_sceneNodeMetaData.ID;
        }

        friend bool operator!=(const SceneNode& lhs, const SceneNode& rhs) {
            return !(lhs == rhs);
        }

    public:
        Transformations* m_transformation;
    protected:
        //=========================
        // Member Variables
        //=========================
        Transformations m_localTransformation;
        SceneNode* m_parent;
        std::shared_ptr<StaticMesh> m_mesh;
        std::string m_name;
        std::vector<std::shared_ptr<SceneNode>> m_children;

        SceneNodeMetaData m_sceneNodeMetaData;
    };

} // namespace ApplicationCore

#endif // SCENENODE_HPP
