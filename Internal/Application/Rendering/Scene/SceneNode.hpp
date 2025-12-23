/*
 * Created by wpsimon09 on 26/11/24.
 */

#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/Structs/ApplicationStructs.hpp"

namespace VulkanUtils {
struct RenderContext;
}

namespace VulkanStructs {
struct VDrawCallData;
struct RenderContext;
}  // namespace VulkanStructs

namespace ApplicationCore {
struct SceneData;

class StaticMesh;
struct DrawContext;
class Transformations;

//=========================
// Static Variables
//=========================
static int SceneNodeIDCounter;

//=========================
// SceneNode Class
//=========================
class SceneNode : public std::enable_shared_from_this<SceneNode>
{
  public:
    //=========================
    // Constructors
    //=========================
    explicit SceneNode(std::shared_ptr<StaticMesh> mesh);
    explicit SceneNode();
    SceneNode(SceneData& sceneData, SceneNode& other);

    //=========================
    // Hierarchy Management
    //=========================
    void AddChild(SceneData& sceneData, std::shared_ptr<SceneNode>& child);
    void AddChild(SceneData& sceneData, std::shared_ptr<StaticMesh> child);
    void AddChild(SceneData& sceneData, std::shared_ptr<StaticMesh> child, std::shared_ptr<BaseMaterial> material);
    SceneNode* GetParent();
    bool       IsParent() const { return m_parent == nullptr; }

    //=========================
    // Visibility Management
    //=========================
    void SetVisibility(bool isVisible);
    bool GetIsVisible() const { return m_sceneNodeMetaData.IsVisible; }

    void SetHiddenInEditor() { m_sceneNodeMetaData.ShowInEditor = false; }
    void SetVisibleInEditor() { m_sceneNodeMetaData.ShowInEditor = true; }
    bool IsVisibleInEditor() const { return m_sceneNodeMetaData.ShowInEditor; }

    //=========================
    // Selection Management
    //=========================
    void SetExpansionState(bool expansionState) { m_sceneNodeMetaData.IsOpen = expansionState; }
    void Select(bool selectedFromWorld = false);
    void Deselect();

    bool               IsSelected() const { return m_sceneNodeMetaData.IsSelected; }
    bool               IsOpen() const { return m_sceneNodeMetaData.IsOpen; }
    SceneNodeMetaData& GetSceneNodeMetaData() { return m_sceneNodeMetaData; }

    //=========================
    // Transformation
    //=========================
    virtual void Update(SceneUpdateContext& sceneUpdateFlags);
    void         SetLocalTransform(Transformations& transformations)
    {
        m_localTransformation = transformations;
        m_transformation      = &m_localTransformation;
    }

    //=========================
    // Rendering
    //=========================
    virtual void Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const;

    //=========================
    // Utility Functions
    //=========================
    void  PreformRayIntersectionTest(Ray& ray, std::vector<std::shared_ptr<SceneNode>>& result);
    bool  HasMesh() const { return m_sceneNodeMetaData.HasMesh; }
    float GetDistanceFromCamera(glm::vec3 cameraPosition);
    bool  IsLight() const;
    bool  IsVolumeNode() const;

    //=========================
    // Accessors
    //=========================
    std::string& GetName() { return m_name; }
    void         SetName(const std::string& name) { m_name = name; }

    std::shared_ptr<StaticMesh>& GetMesh() { return m_mesh; }

    std::vector<std::reference_wrapper<SceneNode>> GetChildrenByWrapper();
    std::shared_ptr<SceneNode>&                    GetChild(int index) { return m_children[index]; }
    std::vector<std::shared_ptr<SceneNode>>&       GetChildrenByRef() { return m_children; }

    //=========================
    // Callback Functions
    //=========================
    virtual void ProcessNodeRemove() {}
    virtual void ProcessNodeRemove(SceneData& sceneData);
    virtual void ProcessNodeRemove(const SceneNode& node, SceneData& sceneData);

    //=========================
    // Operators
    //=========================
    friend bool operator==(const SceneNode& lhs, const SceneNode& rhs)
    {
        return lhs.m_sceneNodeMetaData.ID == rhs.m_sceneNodeMetaData.ID;
    }

    friend bool operator!=(const SceneNode& lhs, const SceneNode& rhs) { return !(lhs == rhs); }

  public:
    Transformations* m_transformation = nullptr;
    glm::mat4        m_prevFrameModelMatrix;

  protected:
    //=========================
    // Member Variables
    //=========================
    Transformations                         m_localTransformation;
    SceneNode*                              m_parent = nullptr;
    std::string                             m_name;
    std::shared_ptr<StaticMesh>             m_mesh;
    std::vector<std::shared_ptr<SceneNode>> m_children;

    std::vector<int> m_childrenIdx;
    int              m_meshIdx     = -1;
    int              m_materialIdx = -1;
    int              m_nodeIndex   = -1;

    SceneNodeMetaData m_sceneNodeMetaData;

  private:
    friend SceneData;
};

}  // namespace ApplicationCore

#endif  // SCENENODE_HPP
