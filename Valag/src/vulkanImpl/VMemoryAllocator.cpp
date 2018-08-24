#include "Valag/vulkanImpl/VMemoryAllocator.h"

#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/utils/Logger.h"

namespace vlg
{


uint32_t VMemoryAllocator::BUFFER_SIZE = 256*1024*1024; //256 mb

VMemoryAllocator::VMemoryAllocator()
{
    //ctor
}

VMemoryAllocator::~VMemoryAllocator()
{
    this->cleanAll();
}

uint32_t VMemoryAllocator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(VInstance::physicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    throw std::runtime_error("Failed to find suitable memory type!");
}

bool VMemoryAllocator::allocBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VBuffer &vbuffer)
{
    return VMemoryAllocator::instance()->allocBufferImpl(size,usage,properties,vbuffer);
}

bool VMemoryAllocator::allocBufferImpl(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VBuffer &vbuffer)
{
    VkDevice device = VInstance::device();

    auto itAllocatedBuffers = m_buffers.find({usage, properties});

    if(itAllocatedBuffers == m_buffers.end())
    {
        this->createBuffer(usage, properties);
        return this->allocBufferImpl(size, usage, properties,vbuffer);
    }

    VkDeviceSize offset = 0;
    AllocatedBuffer allocatedBuffer = {};

    VkDeviceSize alignedSize = size;

    size_t i = 0;
    for(auto &it : itAllocatedBuffers->second)
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, it.buffer, &memRequirements);

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

    vbuffer.usage = usage;
    vbuffer.properties = properties;

    vbuffer.buffer = allocatedBuffer.buffer;
    vbuffer.bufferID = m_currentID++;
    vbuffer.bufferMemory = allocatedBuffer.bufferMemory;
    vbuffer.memoryTypeIndex = allocatedBuffer.memoryTypeIndex;
    vbuffer.offset = offset;

    return (true);
}

bool VMemoryAllocator::copyBuffer(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName)
{
    return VMemoryAllocator::instance()->copyBufferImpl(srcBuffer, dstBuffer, size, commandPoolName);
}

bool VMemoryAllocator::copyBufferImpl(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName)
{
    VkCommandBuffer commandBuffer = VInstance::instance()->beginSingleTimeCommands(commandPoolName);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = srcBuffer.offset;
    copyRegion.size = size;
    copyRegion.dstOffset = dstBuffer.offset;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    VInstance::instance()->endSingleTimeCommands(commandBuffer,commandPoolName);

    return (true);
}

bool VMemoryAllocator::freeBuffer(VBuffer &vbuffer)
{
    return VMemoryAllocator::instance()->freeBufferImpl(vbuffer);
}


bool VMemoryAllocator::freeBufferImpl(VBuffer &vbuffer)
{
    vbuffer.buffer = VK_NULL_HANDLE;

    auto itAllocatedBuffers = m_buffers.find({vbuffer.usage, vbuffer.properties});
    if(itAllocatedBuffers == m_buffers.end())
        return (false);

    AllocatedBuffer &allocatedBuffer = itAllocatedBuffers->second[vbuffer.allocatingBuffer];

    bool alreadyMerged = false;
    auto &lastMerge = *allocatedBuffer.emptyRanges.begin();

    auto leftmostRight = allocatedBuffer.emptyRanges.begin();

    for(auto &it : allocatedBuffer.emptyRanges)
    {
        if(it.first < vbuffer.offset)
            ++leftmostRight;

        if(it.first+it.second == vbuffer.offset)
        {
            it.second += vbuffer.alignedSize;
            alreadyMerged = true;
            lastMerge = it;
        }
        else if(it.first == vbuffer.offset+vbuffer.alignedSize)
        {
            if(alreadyMerged)
            {
                lastMerge.second += it.second;
                allocatedBuffer.emptyRanges.remove(it);
            } else {
                it.first -= vbuffer.alignedSize;
                it.second += vbuffer.alignedSize;
                alreadyMerged =true;
            }
            break;
        }
    }

    if(!alreadyMerged)
        allocatedBuffer.emptyRanges.insert(leftmostRight,{vbuffer.offset, vbuffer.alignedSize});

    return (true);
}

bool VMemoryAllocator::searchForSpace(AllocatedBuffer &allocatedBuffer, VkDeviceSize alignedSize, VkDeviceSize &offset)
{
    for(auto &emptyRange : allocatedBuffer.emptyRanges)
    {
        if(emptyRange.second >= alignedSize)
        {
            offset = emptyRange.first;

            emptyRange.first += alignedSize;
            emptyRange.second -= alignedSize;

            return (true);
        }
    }

    return (false);
}

bool VMemoryAllocator::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkDevice device = VInstance::device();

    VkBuffer        buffer;
    VkDeviceMemory  bufferMemory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = BUFFER_SIZE;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        Logger::error("Failed to allocate buffer memory");
        return (false);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanHelpers::findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        Logger::error("Failed to allocate buffer memory");
        return (false);
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);

    AllocatedBuffer allocatedBuffer = {};
    allocatedBuffer.buffer = buffer;
    allocatedBuffer.bufferMemory = bufferMemory;
    allocatedBuffer.bufferSize = memRequirements.size;
    allocatedBuffer.memoryTypeIndex = allocInfo.memoryTypeIndex;
    allocatedBuffer.emptyRanges.push_back({0, memRequirements.size});

    m_buffers[{usage, properties}].push_back(allocatedBuffer);

    return (true);
}

void VMemoryAllocator::cleanAll()
{
    VkDevice device = VInstance::device();

    for(auto &subBuffer : m_allocatedSubBuffers)
        this->freeBufferImpl(subBuffer.second);
    m_allocatedSubBuffers.clear();

    for(auto bufferList : m_buffers)
    for(auto buffer : bufferList.second)
    {
        vkDestroyBuffer(device, buffer.buffer, nullptr);
        vkFreeMemory(device, buffer.bufferMemory, nullptr);
    }
    m_buffers.clear();
}

}
