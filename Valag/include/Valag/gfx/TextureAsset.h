#ifndef TEXTUREASSET_H
#define TEXTUREASSET_H

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

//#include <glm/glm.hpp>
//#include <vulkan/vulkan.h>

#include "Valag/vulkanImpl/vulkanImpl.h"
#include "Valag/vulkanImpl/VTexture.h"

#include "Valag/core/Asset.h"

namespace vlg
{

class VInstance;

class TextureAsset : public Asset
{
    friend class VApp;

    public:
        TextureAsset();
        TextureAsset(const AssetTypeID&);
        ///TextureAsset(const sf::Image &img);
        ///TextureAsset(sf::Texture *);
        virtual ~TextureAsset();

        bool loadFromFile(const std::string &filePath);
        bool loadFromMemory(void* data, std::size_t dataSize);

        //virtual bool loadNow();

        ///void generateMipmap();
        ///void setSmooth(bool = true);

        ///virtual sf::Texture* GetTexture();

        //VkImageView     getImageView();

        VTexture getVTexture();

    protected:
        //bool generateTexture(unsigned char* pixels, int texWidth, int texHeight);

        ///sf::Texture* m_texture;
        /*VkImage         m_textureImage;
        VkDeviceMemory  m_textureImageMemory;
        VkImageView     m_textureImageView;*/

        VTexture m_vtexture;

        glm::vec2 m_size;

    private:
        bool m_createdTexture;
};

}

#endif // TEXTUREASSET_H
