#include "Valag/vulkanImpl/VBuffersAllocator.h"

#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/utils/Logger.h"

namespace vlg
{


uint32_t VBuffersAllocator::BUFFER_SIZE = 256*1024*1024; //256 mb

VBuffersAllocator::VBuffersAllocator()
{
    //ctor
}

VBuffersAllocator::~VBuffersAllocator()
{
    this->cleanAll();
}

uint32_t VBuffersAllocator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(VInstance::physicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    throw std::runtime_error("Failed to find suitable memory type!");
}

bool VBuffersAllocator::allocBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VBuffer &vbuffer)
{
    return VBuffersAllocator::instance()->allocBufferImpl(size,usage,properties,vbuffer);
}

bool VBuffersAllocator::allocBufferImpl(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VBuffer &vbuffer)
{
    VkDevice device = VInstance::device();

    auto itAllocatedBuffers = m_buffers.find({usage, properties});

    if(itAllocatedBuffers == m_buffers.end())
    {
        this->createBuffer(usage, properties);
        return this->allocBufferImpl(size, usage, properties,vbuffer);
    }

    VkDeviceSize offset = 0;
    AllocatedBuffer *allocatedBuffer = nullptr;

    VkDeviceSize alignedSize = size;

    size_t i = 0;
    for(auto it : itAllocatedBuffers->second)
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, it->buffer, &memRequirements);

        alignedSize = size;
        alignedSize += memRequirements.alignment - (size % memRequirements.alignment);

        if(this->searchForSpace(it,alignedSize,offset) == true)
        {
            allocatedBuffer = it;
            vbuffer.allocatingBuffer = i;
            vbuffer.alignedSize = alignedSize;

            break;
        }
        ++i;
    }

    if(allocatedBuffer == nullptr)
    {
        this->createBuffer(usage, properties);
        return this->allocBufferImpl(size, usage, properties,vbuffer);
    }

    vbuffer.usage = usage;
    vbuffer.properties = properties;

    vbuffer.buffer = allocatedBuffer->buffer;
    vbuffer.bufferMemory = allocatedBuffer->bufferMemory;
    vbuffer.offset = offset;

    return (true);
}

void VBuffersAllocator::writeBuffer(VBuffer dstBuffer, void* data, VkDeviceSize size, bool flush)
{
    VBuffersAllocator::instance()->writeBufferImpl(dstBuffer, data, size,flush);
}

void VBuffersAllocator::writeBufferImpl(VBuffer dstBuffer, void* data, VkDeviceSize size, bool flush)
{
    auto device = VInstance::device();

    void* dstData;

    AllocatedBuffer *allocatedBuffer = this->findAllocatingBuffer(dstBuffer);

    allocatedBuffer->mutex.lock();
    vkMapMemory(device, dstBuffer.bufferMemory, dstBuffer.offset, size, 0, &dstData);
        memcpy(dstData, data, (size_t) size);
        if(flush)
        {
            VkMappedMemoryRange memoryRange = {};
            memoryRange.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memoryRange.memory  = dstBuffer.bufferMemory;
            memoryRange.size    = size;
            memoryRange.offset  = dstBuffer.offset;
            vkFlushMappedMemoryRanges(device, 1, &memoryRange);
        }
    vkUnmapMemory(device, dstBuffer.bufferMemory);
    allocatedBuffer->mutex.unlock();
}

void VBuffersAllocator::copyBuffer(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName)
{
    VkCommandBuffer commandBuffer = VInstance::instance()->beginSingleTimeCommands(commandPoolName);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = srcBuffer.offset;
    copyRegion.size = size;
    copyRegion.dstOffset = dstBuffer.offset;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    VInstance::instance()->endSingleTimeCommands(commandBuffer);
}


void VBuffersAllocator::copyBufferToImage(VBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer,
                                   CommandPoolName cmdPoolName)
{
    VkCommandBuffer commandBuffer = VInstance::instance()->beginSingleTimeCommands(cmdPoolName);

    VkBufferImageCopy region = {};
    region.bufferOffset = buffer.offset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VInstance::instance()->endSingleTimeCommands(commandBuffer);
}

bool VBuffersAllocator::freeBuffer(VBuffer &vbuffer)
{
    return VBuffersAllocator::instance()->freeBufferImpl(vbuffer);
}


bool VBuffersAllocator::freeBufferImpl(VBuffer &vbuffer)
{
    AllocatedBuffer *allocatedBuffer = this->findAllocatingBuffer(vbuffer);

    if(allocatedBuffer == nullptr)
        return (false);

    bool alreadyMerged = false;
    auto lastMerge = allocatedBuffer->emptyRanges.begin();

    auto leftmostRight = allocatedBuffer->emptyRanges.begin();

    for(auto it = allocatedBuffer->emptyRanges.begin() ; it != allocatedBuffer->emptyRanges.end() ; ++it)
    {
        if(it->first < vbuffer.offset)
            ++leftmostRight;


        if(it->first+it->second == vbuffer.offset)
        {
            it->second += vbuffer.alignedSize;
            alreadyMerged = true;
            lastMerge = it;
        }
        else if(it->first == vbuffer.offset+vbuffer.alignedSize)
        {
            if(alreadyMerged)
            {
                lastMerge->second += it->second;
                allocatedBuffer->emptyRanges.erase(it);
            } else {
                it->first -= vbuffer.alignedSize;
                it->second += vbuffer.alignedSize;
                alreadyMerged = true;
            }
            break;
        }
    }

    if(!alreadyMerged)
        allocatedBuffer->emptyRanges.insert(leftmostRight,{vbuffer.offset, vbuffer.alignedSize});

    vbuffer.buffer = VK_NULL_HANDLE;

    return (true);
}

bool VBuffersAllocator::searchForSpace(AllocatedBuffer *allocatedBuffer, VkDeviceSize alignedSize, VkDeviceSize &offset)
{
    for(auto &emptyRange : allocatedBuffer->emptyRanges)
    {
        if(emptyRange.second >= alignedSize)
        {
            offset = emptyRange.first;

            emptyRange.first += alignedSize;
            emptyRange.second -= alignedSize;

            if(emptyRange.second == 0)
                allocatedBuffer->emptyRanges.remove(emptyRange);

            return (true);
        }
    }

    return (false);
}

bool VBuffersAllocator::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkBuffer        buffer;
    VkDeviceMemory  bufferMemory;

    VulkanHelpers::createBuffer(BUFFER_SIZE,usage,properties,buffer,bufferMemory);

    AllocatedBuffer *allocatedBuffer = new AllocatedBuffer();
    allocatedBuffer->buffer = buffer;
    allocatedBuffer->bufferMemory = bufferMemory;
    allocatedBuffer->bufferSize = BUFFER_SIZE;
    allocatedBuffer->emptyRanges.push_back({0, BUFFER_SIZE});

    m_buffers[{usage, properties}].push_back(allocatedBuffer);

    return (true);
}

AllocatedBuffer *VBuffersAllocator::findAllocatingBuffer(VBuffer &vbuffer)
{
    auto itAllocatedBuffers = m_buffers.find({vbuffer.usage, vbuffer.properties});
    if(itAllocatedBuffers == m_buffers.end())
        return nullptr;

    return itAllocatedBuffers->second[vbuffer.allocatingBuffer];
}

void VBuffersAllocator::cleanAll()
{
    VkDevice device = VInstance::device();

    for(auto bufferList : m_buffers)
    for(auto buffer : bufferList.second)
    {
        vkDestroyBuffer(device, buffer->buffer, nullptr);
        vkFreeMemory(device, buffer->bufferMemory, nullptr);
        delete buffer;
    }
    m_buffers.clear();
}

}
