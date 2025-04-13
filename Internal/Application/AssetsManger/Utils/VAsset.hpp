#ifndef ASSET_HPP
#define ASSET_HPP

#include <unordered_map>
#include <memory>
#include <future>
#include <optional>
#include <filesystem>

#include "Vulkan/Global/GlobalStructs.hpp"

namespace VulkanCore {
class VDevice;
}

namespace ApplicationCore {

template <typename T>
class VAsset
{
public:
  explicit VAsset(const VulkanCore::VDevice& device);

  //===================================
  // GETTERS
  //===================================
  virtual std::shared_ptr<T> GetHandle() { return m_deviceHandle; }
  virtual T&                 GetHandleByRef() { return *m_deviceHandle; }
  std::string&               GetName() { return m_name; }
  std::filesystem::path&     GetAssetPath();
  bool                       IsSavable() { return m_savable; }
  void                       AllowSave() { m_savable = true; }
  void                       DisallowSave() { m_savable = false; }

  //===================================
  // PUBLIC VIRTUAL METHODS
  //===================================
  virtual void Load() { LoadInternal(); }

  virtual bool Sync()    = 0;
  virtual void Destroy() = 0;

  static const int ID;

protected:
  virtual void LoadInternal() = 0;

  std::optional<std::filesystem::path> m_assetPath;
  std::string                          m_name;
  bool                                 m_isLoaded;
  bool                                 m_savable  = false;
  bool                                 m_isInSync = true;

  std::future<std::shared_ptr<T>> m_futureDeviceHandle;
  std::shared_ptr<T>              m_deviceHandle;
  const VulkanCore::VDevice&      m_device;
};
//================================================================================================
// END OF HEADER
//================================================================================================
//================================================================================================

//=================================
// DEFINITIONS
//=================================
template <typename T>
inline VAsset<T>::VAsset(const VulkanCore::VDevice& device)
    : m_device(device)
{
}

template <typename T>
std::filesystem::path& VAsset<T>::GetAssetPath()
{
  if(m_assetPath.has_value())
    return m_assetPath.value();
  else
    throw std::runtime_error("AssetPath is empty, make sure you are assigning the path in sync function");
}
}  // namespace ApplicationCore

#endif  // ASSET_HPP
