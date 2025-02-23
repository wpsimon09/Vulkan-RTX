#ifndef ASSET_HPP
#define ASSET_HPP

#include <unordered_map>
#include <memory>
#include <future>
#include <optional>
#include <filesystem>

namespace VulkanCore{
    class VDevice;
}

namespace ApplicationCore{

    template<typename T>
    class VAsset {
    public:
        explicit VAsset(const VulkanCore::VDevice& device);
    
        //===================================
        // GETTERS 
        //===================================
        std::shared_ptr<T>GetHandle() {return m_deviceHandle;}
        T& GetHandleByRef() {return *m_deviceHandle;}
        std::string& GetName() {return m_name;}
        std::string& GetAssetPath() {return m_name;}
        bool IsSavable() {return m_savable;}
        void AllowSave() {m_savable = true;}
        void DisallowSave() {m_savable = false;}

        //===================================
        // PUBLIC VIRTUAL METHODS
        //===================================
        void Load() {LoadInternal();};
        
        virtual void Sync() = 0;
        virtual void Destroy() = 0;

        static const int ID;
    protected:
        virtual void LoadInternal() = 0;

    protected:
        std::optional<std::filesystem::path> m_assetPath;
        std::string m_name;
        bool m_isLoaded;
        bool m_savable = false;

        std::future<std::shared_ptr<T>> m_futureDeviceHandle;
        std::shared_ptr<T> m_deviceHandle;
        const VulkanCore::VDevice& m_device;
        
    };
    //================================================================================================
    // END OF HEADER
    //================================================================================================
    //================================================================================================

    //=================================
    // DEFINITIONS
    //=================================
    template <typename T>
    inline VAsset<T>::VAsset(const VulkanCore::VDevice &device): m_device(device)
    {
        
    }
}

#endif // ASSET_HPP
