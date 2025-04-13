//
// Created by wpsimon09 on 02/10/24.
//

#ifndef VKOBJECT_HPP
#define VKOBJECT_HPP

inline int globalIDCounter;

namespace VulkanCore {
class VObject
{
public:
  VObject();
  virtual void Destroy() {};
  virtual ~VObject() = default;

  VObject(const VObject&)            = delete;
  VObject& operator=(const VObject&) = delete;


public:
  const int GetID() const { return m_ID; }

private:
  int m_ID;

  friend bool operator==(const VObject& lhs, const VObject& rhs) { return lhs.m_ID == rhs.m_ID; }

  friend bool operator!=(const VObject& lhs, const VObject& rhs) { return !(lhs == rhs); }
};
}  // namespace VulkanCore


#endif  //VKOBJECT_HPP
