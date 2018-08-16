#include "Valag/utils/DynamicUBO.h"

#include "Vulkan/vulkan.h"


/// Inspiration from : https://github.com/SaschaWillems/Vulkan/tree/master/examples/dynamicuniformbuffer



namespace vlg
{

DynamicUBO::DynamicUBO(size_t objectSize, size_t chunkSize ) :
    m_objectSize(objectSize),
    m_chunkSize(chunkSize)
{
    m_firstTime = true;
    m_totalSize = 0;
    this->computeDynamicAlignment();
    this->expandBuffers();
}

DynamicUBO::~DynamicUBO()
{
    this->cleanup();
}

bool DynamicUBO::allocObject(size_t &index)
{
    bool r = false;
    if(m_availableIndices.empty())
    {
        this->expandBuffers();
        r = true;
    }

    index = *m_availableIndices.begin();
    m_availableIndices.pop_front();

    return r;
}

bool DynamicUBO::freeObject(size_t index)
{
    m_availableIndices.push_back(index);

    return (true);
}

bool DynamicUBO::updateObject(size_t index, void *newData)
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No Vulkan instance in DynamicUBO::updateObject()");

    VkDevice device = vulkanInstance->getDevice();

    void* data;
    vkMapMemory(device, m_bufferMemory, this->getDynamicOffset(index), m_dynamicAlignment, 0, &data);
        memcpy(data, newData, m_objectSize);

    VkMappedMemoryRange memoryRange = {};
    memoryRange.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memoryRange.memory  = m_bufferMemory;
    memoryRange.size    = m_dynamicAlignment;
    memoryRange.offset  = this->getDynamicOffset(index);
    vkFlushMappedMemoryRanges(vulkanInstance->getDevice(), 1, &memoryRange);

    vkUnmapMemory(device, m_bufferMemory);

    return (true);
}

uint32_t DynamicUBO::getDynamicOffset(size_t index)
{
    return index * static_cast<uint32_t>(m_dynamicAlignment);
}

VkBuffer DynamicUBO::getBuffer()
{
    return m_buffer;
}

void DynamicUBO::computeDynamicAlignment()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No Vulkan instance in DynamicUBO::createBuffers()");

    VkPhysicalDeviceProperties  deviceProperties = {};
    vkGetPhysicalDeviceProperties(vulkanInstance->getPhysicalDevice(), &deviceProperties);

    size_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;

	m_dynamicAlignment = m_objectSize;
	if (minUboAlignment > 0)
		m_dynamicAlignment = (m_dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
}

void DynamicUBO::createBuffers()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No Vulkan instance in DynamicUBO::createBuffers()");

    m_bufferSize = m_totalSize * m_dynamicAlignment;
    VulkanHelpers::createBuffer(m_bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                                ,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                m_buffer, m_bufferMemory);

}

void DynamicUBO::expandBuffers()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();
    VkDevice device = vulkanInstance->getDevice();

    VkBuffer        oldBuffer = m_buffer;
    VkDeviceMemory  oldbufferMemory = m_bufferMemory;
    VkDeviceSize    oldBufferSize = m_bufferSize;

    for(size_t i = 0 ; i < m_chunkSize ; ++i)
        m_availableIndices.push_back(m_totalSize+i);
    m_totalSize += m_chunkSize;
    this->createBuffers();

    if(!m_firstTime)
    {
        VulkanHelpers::copyBuffer(m_buffer, oldBuffer, oldBufferSize);

        vkDestroyBuffer(device, oldBuffer, nullptr);
        vkFreeMemory(device, oldbufferMemory, nullptr);
    }

    m_firstTime = false;
}

void DynamicUBO::cleanup()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();
    VkDevice device = vulkanInstance->getDevice();

    vkDestroyBuffer(device, m_buffer, nullptr);
    vkFreeMemory(device, m_bufferMemory, nullptr);
}


}
