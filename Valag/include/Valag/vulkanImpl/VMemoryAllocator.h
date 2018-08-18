#ifndef VMEMORYALLOCATOR_H
#define VMEMORYALLOCATOR_H

#include <Vulkan/Vulkan.h>
#include <glm/glm.hpp>
#include <map>
#include <vector>

#include "Valag/Types.h"
#include "Valag/utils/Singleton.h"

namespace vlg
{

struct VBuffer
{
    VBuffer() : buffer(VK_NULL_HANDLE) {}

    VkBuffer                buffer;
    VkDeviceMemory          bufferMemory;
    VkDeviceSize            offset;
    uint32_t                memoryTypeIndex;
    VBufferID               bufferID;
};

struct AllocatedBuffer
{
    uint32_t                memoryTypeIndex;
    VkBuffer                buffer;
    VkDeviceMemory          bufferMemory;
    VkDeviceSize            bufferSize;
};

class VMemoryAllocator : public Singleton<VMemoryAllocator>
{
    public:
        friend class Singleton<VMemoryAllocator>;
        friend class VApp;

        static bool allocBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VBuffer &vbuffer);
        static bool copyBuffer(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);
        static bool freeBuffer(VBuffer &vbuffer);

    protected:
        VMemoryAllocator();
        virtual ~VMemoryAllocator();

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        bool freeBufferImpl(VBuffer &vbuffer);
        bool copyBufferImpl(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName);
        bool allocBufferImpl(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VBuffer &vbuffer);

        void cleanAll();

    private:
        std::map<uint32_t, std::vector<AllocatedBuffer>> m_buffers;

        std::map<VBufferID, VBuffer>    m_allocatedSubBuffers;
        VBufferID                       m_currentID;

};

}

#endif // VMEMORYALLOCATOR_H
