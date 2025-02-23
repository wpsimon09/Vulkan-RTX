
#ifndef VTEXTUREASSET_HPP
#define VTEXTUREASSET_HPP

#include "VAsset.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"

struct TextureBufferInfo;

namespace ApplicationCore {
    class VTextureAsset : public VAsset<VulkanCore::VImage> {
    public:
        explicit VTextureAsset(const VulkanCore::VDevice& device, std::filesystem::path texturePath);
        explicit VTextureAsset(const VulkanCore::VDevice& device, TextureBufferInfo& bufferInfo);


        void Sync() override;
        void Destroy() override;
    protected:
        void LoadInternal() override;
        void LoadInternalFromBuffer();
    private:
        EImageSource m_textureSource;
        int m_width;
        int m_height;
        int m_mipLevels;
        
    };

}

#endif // VTEXTUREASSET_HPP
