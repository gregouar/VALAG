#ifndef VTEXTURE_H
#define VTEXTURE_H

#include <vulkan/vulkan.h>
#include "Valag/Types.h"

namespace vlg
{

class VTexture
{
    friend class VTexturesManager;

    public:
        VTexture();
        virtual ~VTexture();

        bool generateTexture(int texWidth, int texHeight,
                             unsigned char* pixels, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);
        bool generateTexture(int texWidth, int texHeight, VkFormat format,
                             unsigned char* pixels, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);

        uint32_t    getTextureId();
        uint32_t    getTextureLayer();
        glm::vec2   getTexturePair();
        VkExtent2D  getExtent();
        VkFormat    getFormat();

    protected:
        uint32_t m_textureId;
        uint32_t m_textureLayer;

        VkExtent2D  m_extent;
        VkFormat    m_format;

};

}

#endif // VTEXTURE_H
