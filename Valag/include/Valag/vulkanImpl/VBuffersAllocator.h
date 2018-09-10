#ifndef VBUFFERSALLOCATOR_H
#define VBUFFERSALLOCATOR_H

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
    VkDeviceSize            alignedSize;

    VkBufferUsageFlags      usage;
    VkMemoryPropertyFlags   properties;
    size_t                  allocatingBuffer;
};

struct AllocatedBuffer
{
    VkBuffer                buffer;
    VkDeviceMemory          bufferMemory;
    VkDeviceSize            bufferSize;

    std::mutex              mutex;

    std::list<std::pair<VkDeviceSize, VkDeviceSize> > emptyRanges; //<offset, size>
};

class VBuffersAllocator : public Singleton<VBuffersAllocator>
{
    public:
        friend class Singleton<VBuffersAllocator>;
        friend class VApp;

        static bool allocBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VBuffer &vbuffer);
        static void writeBuffer(VBuffer dstBuffer, void* data, VkDeviceSize size);
        static void copyBuffer(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);
        static void copyBufferToImage(VBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer,
                                       CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);
        static bool freeBuffer(VBuffer &vbuffer);

    protected:
        VBuffersAllocator();
        virtual ~VBuffersAllocator();

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        bool freeBufferImpl(VBuffer &vbuffer);
        bool allocBufferImpl(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VBuffer &vbuffer);
        void writeBufferImpl(VBuffer dstBuffer, void* data, VkDeviceSize size);

        bool searchForSpace(AllocatedBuffer *allocatedBuffer, VkDeviceSize size, VkDeviceSize &offset);
        bool createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

        AllocatedBuffer *findAllocatingBuffer(VBuffer &buffer);

        void cleanAll();

    private:
        std::map<std::pair<VkBufferUsageFlags, VkMemoryPropertyFlags>, std::vector<AllocatedBuffer*>> m_buffers;

    public:
        static uint32_t BUFFER_SIZE;
};

}

#endif // VBUFFERSALLOCATOR_H
