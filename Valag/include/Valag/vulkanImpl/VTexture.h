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

        bool generateTexture(unsigned char* pixels, int texWidth, int texHeight, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);

        uint32_t    getTextureId();
        uint32_t    getTextureLayer();
        VkExtent2D  getExtent();

    protected:
        uint32_t m_textureId;
        uint32_t m_textureLayer;

        VkExtent2D m_extent;

};

}

#endif // VTEXTURE_H
