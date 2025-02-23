#ifndef ASSET_HPP
#define ASSET_HPP

#include <unordered_map>
#include <memory>
#include <future>
#include <optional>

namespace VulkanCore{
    class VDevice;
}

namespace ApplicationCore{
    template<typename T>
    class VAsset {
    public:
        explicit VAsset(const VDevice& device);
    
        //===================================
        // GETTERS 
        //===================================
        T& GetHandle() {return *m_deviceHandle;}
        std::reference_wrapper<T> GetHandle() {return std::ref<T>(m_handle)}
        std::string& GetName() {return m_name;}
        std::string& GetAssetPath() {return m_name;}

        //===================================
        // VIRTUAL PUBLIC  
        //===================================
        virtual void Load() = 0;
        
        static const int ID;
    protected:
        virtual std::future<std::unique_ptr<T>> LoadInternal() = 0
    
    private:
        std::optional<std::filesystem::path> m_assetPath;
        std::string m_name;
        std::unique_ptr<T> m_deviceHandle;
        
    };
}

#endif // ASSET_HPP

