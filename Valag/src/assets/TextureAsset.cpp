#include "Valag/assets/AssetHandler.h"
#include "Valag/assets/TextureAsset.h"

#include "Valag/utils/Logger.h"

#include "Valag/vulkanImpl/VulkanImpl.h"

//#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace vlg
{

TextureAsset::TextureAsset() : TextureAsset(-1)
{
}

TextureAsset::TextureAsset(const AssetTypeId id) : Asset(id)
{
    m_allowLoadFromFile = true;
    m_allowLoadFromMemory = false;
    ///m_allowLoadFromStream = true;

   /// m_texture->setRepeated(true);
   /// m_texture->setSmooth(true);
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
}

bool TextureAsset::loadFromFile(const std::string &filePath)
{
    int texWidth, texHeight, texChannels;

    CommandPoolName commandPoolName;
    if(m_loadType == LoadType_Now)
        commandPoolName = COMMANDPOOL_SHORTLIVED;
    else
        commandPoolName = COMMANDPOOL_TEXTURESLOADING;

    if(stbi_is_hdr(filePath.c_str()))
    {
        float* pixels = stbi_loadf(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            Logger::error("Cannot load texture from file: "+m_filePath);
            return (false);
        }

        m_vtexture.generateTexture(texWidth, texHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                   (unsigned char*)pixels,commandPoolName);
        stbi_image_free(pixels);
    } else {
        stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            Logger::error("Cannot load texture from file: "+m_filePath);
            return (false);
        }

        m_vtexture.generateTexture(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM,
                                   pixels,commandPoolName);
        stbi_image_free(pixels);
    }

    Logger::write("Texture loaded from file: "+filePath);

    return (true);
}

bool TextureAsset::loadFromMemory(void *data, std::size_t dataSize)
{
    return (false);
}

VTexture TextureAsset::getVTexture()
{
    return m_vtexture;
}

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
