#include "Valag/vulkanImpl/VTexture.h"

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

VTexture::VTexture()
{
    m_textureId = 0;
    m_textureLayer = 0;
}

VTexture::~VTexture()
{
    //dtor
}


bool VTexture::generateTexture(unsigned char* pixels, int texWidth, int texHeight, CommandPoolName commandPoolName)
{
    VBuffer stagingBuffer;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VBuffersAllocator::allocBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer);

    VBuffersAllocator::writeBuffer(stagingBuffer, pixels, static_cast<size_t>(imageSize));

    if(!VTexturesManager::allocTexture(texWidth,texHeight, stagingBuffer,commandPoolName,this))
        return (false);

    VBuffersAllocator::freeBuffer(stagingBuffer);

    m_extent.width = texWidth;
    m_extent.height = texHeight;

    return (true);
}

uint32_t VTexture::getTextureId()
{
    return m_textureId;
}

uint32_t VTexture::getTextureLayer()
{
    return m_textureLayer;
}

VkExtent2D VTexture::getExtent()
{
    return m_extent;
}



}
