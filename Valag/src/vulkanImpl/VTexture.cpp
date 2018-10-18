#include "Valag/vulkanImpl/VTexture.h"

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

VTexture::VTexture()
{
    m_textureId     = 0;
    m_textureLayer  = 0;
}

VTexture::~VTexture()
{
    //dtor
}

bool VTexture::generateTexture(int texWidth, int texHeight, unsigned char* pixels,
                               CommandPoolName commandPoolName)
{
    return this->generateTexture(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, pixels, commandPoolName);
}

bool VTexture::generateTexture(int texWidth, int texHeight, VkFormat format,
                            unsigned char* pixels,CommandPoolName commandPoolName)
{
    VBuffer stagingBuffer;
    VkDeviceSize imageSize = texWidth * texHeight;

    if(format == VK_FORMAT_R8G8B8A8_UNORM)
        imageSize *= 4;
    else if(format == VK_FORMAT_R16G16B16A16_SFLOAT)
        imageSize *= 8;
    else if(format == VK_FORMAT_R32G32B32A32_SFLOAT)
        imageSize *= 16;
    ///Could add others here

    VBuffersAllocator::allocBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer);

    VBuffersAllocator::writeBuffer(stagingBuffer, pixels, static_cast<size_t>(imageSize));

    if(!VTexturesManager::allocTexture(texWidth, texHeight, format, stagingBuffer,commandPoolName,this))
        return (false);

    VBuffersAllocator::freeBuffer(stagingBuffer);

    m_extent.width  = texWidth;
    m_extent.height = texHeight;
    m_format = format;

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

glm::vec2 VTexture::getTexturePair()
{
    return {m_textureId, m_textureLayer};
}

VkExtent2D VTexture::getExtent()
{
    return m_extent;
}

VkFormat VTexture::getFormat()
{
    return m_format;
}



}
