#ifndef TEXTUREASSET_H
#define TEXTUREASSET_H

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

//#include <glm/glm.hpp>
//#include <vulkan/vulkan.h>

#include "Valag/vulkanImpl/vulkanImpl.h"
#include "Valag/vulkanImpl/VTexture.h"

#include "Valag/assets/Asset.h"

namespace vlg
{

class VInstance;

class TextureAsset : public Asset
{
    friend class VApp;

    public:
        TextureAsset();
        TextureAsset(const AssetTypeId);
        virtual ~TextureAsset();

        bool loadFromFile(const std::string &filePath);
        bool loadFromMemory(void* data, std::size_t dataSize);

        ///void generateMipmap();
        ///void setSmooth(bool = true);



        VTexture getVTexture();

    protected:
        VTexture m_vtexture;

        glm::vec2 m_size;
};

}

#endif // TEXTUREASSET_H
