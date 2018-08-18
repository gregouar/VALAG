#include "Valag/vulkanImpl/VMemoryAllocator.h"

#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

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
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("Failed to find suitable memory type");

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkanInstance->getPhysicalDevice(), &memProperties);

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
     VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
    {
        Logger::error("Cannot allocate buffer memory without Vulkan instance");
        return (false);
    }

    if(!vulkanInstance->isInitialized())
    {
        Logger::error("Cannot allocate buffer memory with unitialized Vulkan instance");
        return (false);
    }

    VkDevice device = vulkanInstance->getDevice();

    VkBuffer        buffer;
    VkDeviceMemory  bufferMemory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
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

    m_buffers[allocInfo.memoryTypeIndex].push_back(allocatedBuffer);

    vbuffer.buffer = buffer;
    vbuffer.bufferID = m_currentID++;
    vbuffer.bufferMemory = bufferMemory;
    vbuffer.memoryTypeIndex = allocInfo.memoryTypeIndex;
    vbuffer.offset = 0;

    return (true);
}

bool VMemoryAllocator::copyBuffer(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName)
{
    return VMemoryAllocator::instance()->copyBufferImpl(srcBuffer, dstBuffer, size, commandPoolName);
}

bool VMemoryAllocator::copyBufferImpl(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName)
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No Vulkan instance in copyBufferToImage()");

    VkCommandBuffer commandBuffer = vulkanInstance->beginSingleTimeCommands(commandPoolName);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = srcBuffer.offset;
    copyRegion.size = size;
    copyRegion.dstOffset = dstBuffer.offset;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    vulkanInstance->endSingleTimeCommands(commandBuffer,commandPoolName);

    return (true);
}

bool VMemoryAllocator::freeBuffer(VBuffer &vbuffer)
{
    return VMemoryAllocator::instance()->freeBufferImpl(vbuffer);
}


bool VMemoryAllocator::freeBufferImpl(VBuffer &vbuffer)
{
    vbuffer.buffer = VK_NULL_HANDLE;

    return (true);
}

void VMemoryAllocator::cleanAll()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();
    VkDevice device = vulkanInstance->getDevice();

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
