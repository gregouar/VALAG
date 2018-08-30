#ifndef VTEXTURE_H
#define VTEXTURE_H

#include <vulkan/vulkan.h>
#include "Valag/Types.h"

namespace vlg
{

/// I'LL NEED TO ENCAPSULATE IT

class VTexture
{
    public:
        VTexture();
        virtual ~VTexture();

        bool generateTexture(unsigned char* pixels, int texWidth, int texHeight, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);

        size_t m_textureId;
        size_t m_textureLayer;

        VkExtent2D m_extent;

    protected:

    private:
};

}

#endif // VTEXTURE_H
