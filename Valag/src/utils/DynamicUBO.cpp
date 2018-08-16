#include "Valag/utils/DynamicUBO.h"

#include "Vulkan/vulkan.h"


/// Source : https://github.com/SaschaWillems/Vulkan/tree/master/examples/dynamicuniformbuffer

void* alignedAlloc(size_t size, size_t alignment)
{
	void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}

void alignedFree(void* data)
{
#if	defined(_MSC_VER) || defined(__MINGW32__)
	_aligned_free(data);
#else
	free(data);
#endif
}


namespace vlg
{

DynamicUBO::DynamicUBO(size_t objectSize, size_t chunkSize )
{
    m_currentIndex = 0;
    this->createBuffers(objectSize, chunkSize);
}

DynamicUBO::~DynamicUBO()
{
    this->cleanup();
}

bool DynamicUBO::allocObject(size_t &index)
{
    bool r = false;
    if(m_currentIndex > m_totalSize)
    {
        this->expandBuffers();
        r = true;
    }

    index = m_currentIndex++;

    return r;
}

bool DynamicUBO::freeObject(size_t index)
{
    return (true);
}

bool DynamicUBO::updateObject(size_t index, void *newData)
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No Vulkan instance in DynamicUBO::updateObject()");

    VkDevice device = vulkanInstance->getDevice();

    // Aligned offset
   // void* dataOffset = (void*)(((uint64_t)m_localData + (index * m_dynamicAlignment)));
    //*dataOffset = *newData;

    //memcpy(uniformBuffers.dynamic.mapped, data, m_objectSize);


   /* void* data;
    vkMapMemory(device, m_bufferMemory, 0, m_dynamicAlignment, 0, &data);
        memcpy(data, dataOffset, m_dynamicAlignment);
    vkUnmapMemory(device, m_bufferMemory);*/

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

VkDeviceSize DynamicUBO::getBufferRange()
{
    return m_bufferRange;
}

void DynamicUBO::createBuffers(size_t objectSize, size_t chunkSize)
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No Vulkan instance in DynamicUBO::createBuffers()");

    VkPhysicalDeviceProperties  deviceProperties = {};
    vkGetPhysicalDeviceProperties(vulkanInstance->getPhysicalDevice(), &deviceProperties);

    size_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;

    m_objectSize = objectSize;

	m_dynamicAlignment = objectSize;
	if (minUboAlignment > 0)
		m_dynamicAlignment = (m_dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);

    m_chunkSize = chunkSize;
    m_totalSize = m_chunkSize;

    size_t bufferSize = m_chunkSize * m_dynamicAlignment;
    m_bufferRange = bufferSize;
    /*localData = alignedAlloc(bufferSize, m_dynamicAlignment);
    assert(localData);*/

    VulkanHelpers::createBuffer(bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                m_buffer, m_bufferMemory);

}

void DynamicUBO::expandBuffers()
{
    ///INCREASE m_totalSize by m_chunkSize
}

void DynamicUBO::cleanup()
{
    //alignedFree(m_localData);

    //vkDestroyBuffer()
    //vkFreeMemory()
}


}
