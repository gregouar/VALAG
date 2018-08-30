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
    VkDevice device = VInstance::device();

    VBuffer stagingBuffer;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VBuffersAllocator::allocBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer);

    /*void* data;
    vkMapMemory(device, stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBuffer.bufferMemory);*/

    VBuffersAllocator::writeBuffer(stagingBuffer, pixels, static_cast<size_t>(imageSize));

    if(!VTexturesManager::allocTexture(texWidth,texHeight, stagingBuffer,commandPoolName,this))
        return (false);

    VBuffersAllocator::freeBuffer(stagingBuffer);

    m_extent.width = texWidth;
    m_extent.height = texHeight;

    return (true);
}


}
