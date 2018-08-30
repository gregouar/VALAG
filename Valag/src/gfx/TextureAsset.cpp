#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"
#include "Valag/utils/Logger.h"

#include "Valag/vulkanImpl/VulkanImpl.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace vlg
{

TextureAsset::TextureAsset()
{
    ///m_texture = new sf::Texture();
    m_createdTexture = true;

    m_allowLoadFromFile = true;
    m_allowLoadFromMemory = true;
    ///m_allowLoadFromStream = true;

    ///m_texture->setRepeated(true);
    ///m_texture->setSmooth(true);

    //m_textureImage = VK_NULL_HANDLE;
}

TextureAsset::TextureAsset(const AssetTypeID& id) : Asset(id)
{
    ///m_texture = new sf::Texture();
    m_createdTexture = true;

    m_allowLoadFromFile = true;
    m_allowLoadFromMemory = true;
    ///m_allowLoadFromStream = true;

   /// m_texture->setRepeated(true);
   /// m_texture->setSmooth(true);

    //m_textureImage = VK_NULL_HANDLE;
}


/**TextureAsset::TextureAsset(const sf::Image &img) : TextureAsset()
{
    m_texture->loadFromImage(img);
    m_loaded = true;
}

TextureAsset::TextureAsset(sf::Texture *texture)
{
    m_texture = texture;
    if(m_texture != nullptr)
        m_loaded = true;

    m_createdTexture = false;
}**/


TextureAsset::~TextureAsset()
{
    ///if(m_createdTexture && m_texture != nullptr)
       /// delete m_texture;

    VTexturesManager::freeTexture(m_vtexture);

    /*if(m_textureImage != VK_NULL_HANDLE)
    {
        VkDevice device = VInstance::device();

        vkDestroyImageView(device, m_textureImageView, nullptr);
        vkDestroyImage(device, m_textureImage, nullptr);
        vkFreeMemory(device, m_textureImageMemory, nullptr);
    }*/

}

/*bool TextureAsset::generateTexture(stbi_uc* pixels, int texWidth, int texHeight)
{
    VkDevice device = VInstance::device();

    CommandPoolName commandPoolName;

    if(m_loadType == LoadType_Now)
        commandPoolName = COMMANDPOOL_SHORTLIVED;
    else
        commandPoolName = COMMANDPOOL_TEXTURESLOADING;

    //VkBuffer stagingBuffer;
    //VkDeviceMemory stagingBufferMemory;
    VBuffer stagingBuffer;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    //VulkanHelpers::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      //                          stagingBuffer, stagingBufferMemory);
    VBuffersAllocator::allocBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer);

    void* data;
    vkMapMemory(device, stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBuffer.bufferMemory);



    //if(!VTexturesManager::allocTexture(texWidth,texHeight, stagingBuffer,commandPoolName,m_vtexture))
      //  return (false);


    VulkanHelpers::createImage(texWidth, texHeight,1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               m_textureImage, m_textureImageMemory);

    VulkanHelpers::transitionImageLayout(m_textureImage,0, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,commandPoolName);
        VulkanHelpers::copyBufferToImage(stagingBuffer.buffer, m_textureImage,
                                         static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),0,commandPoolName);
    VulkanHelpers::transitionImageLayout(m_textureImage,0, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,commandPoolName);


    //vkDestroyBuffer(device, stagingBuffer, nullptr);
    //vkFreeMemory(device, stagingBufferMemory, nullptr);
    VBuffersAllocator::freeBuffer(stagingBuffer);

    m_size.x = texWidth;
    m_size.y = texHeight;

    m_textureImageView = VulkanHelpers::createImageView(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, 1);

    return (true);
}*/

bool TextureAsset::loadFromFile(const std::string &filePath)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!pixels)
    {
        Logger::error("Cannot load texture from file: "+m_filePath);
        return (false);
    }

   // this->generateTexture(pixels, texWidth, texHeight);


    CommandPoolName commandPoolName;
    if(m_loadType == LoadType_Now)
        commandPoolName = COMMANDPOOL_SHORTLIVED;
    else
        commandPoolName = COMMANDPOOL_TEXTURESLOADING;

    m_vtexture.generateTexture(pixels, texWidth, texHeight,commandPoolName);

    stbi_image_free(pixels);

    Logger::write("Texture loaded from file: "+filePath);

    return (true);
}

bool TextureAsset::loadFromMemory(void *data, std::size_t dataSize)
{
    return (false);
}

/*VkImageView TextureAsset::getImageView()
{
    return m_textureImageView;
}*/

VTexture TextureAsset::getVTexture()
{
    return m_vtexture;
}

/**bool TextureAsset::loadNow()
{
    bool loaded = true;

    if(!m_loaded) {
        if(m_loadSource == LoadSource_File)
        {
            if(!m_texture->loadFromFile(m_filePath))
            {
                Logger::Error("Cannot load texture from file: "+m_filePath);
                loaded = false;
            } else
                Logger::write("Texture loaded from file: "+m_filePath);
        } else if(m_loadSource == LoadSource_Memory) {
            if(!m_texture->loadFromMemory(m_loadData,m_loadDataSize))
            {
                Logger::Error("Cannot load texture from memory");
                loaded = false;
            }
        } else if(m_loadSource == LoadSource_Stream) {
            if(!m_texture->loadFromStream(*m_loadStream))
            {
                Logger::Error("Cannot load texture from stream");
                loaded = false;
            }
        } else {
            Logger::error("Cannot load asset");
            m_loaded = false;
        }

        m_loaded = loaded;
    }

    return Asset::loadNow();
}**/

/**void TextureAsset::generateMipmap()
{
    if(m_texture != nullptr)
        m_texture->generateMipmap();
}

void TextureAsset::SetSmooth(bool smooth)
{
    if(m_texture != nullptr)
        m_texture->setSmooth(smooth);
}

sf::Texture *TextureAsset::GetTexture()
{
    if(m_loaded)
        return m_texture;

    if(AssetHandler<TextureAsset>::Instance()->GetDummyAsset() != nullptr)
        return AssetHandler<TextureAsset>::Instance()->GetDummyAsset()->GetTexture();
    else
        return nullptr;
}**/

}
