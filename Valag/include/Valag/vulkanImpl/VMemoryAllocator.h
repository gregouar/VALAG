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

struct VMemory
{
    VMemory() : vkMemory(VK_NULL_HANDLE){}

    VkDeviceMemory          vkMemory;
    VkDeviceSize            offset;

    VkMemoryRequirements    memRequirements;
    uint32_t                memoryTypeIndex;
    size_t                  allocatingMemory;
};

struct AllocatedMemory
{
    VkDeviceMemory          vkMemory;
    VkDeviceSize            memorySize;

    std::list<std::pair<VkDeviceSize, VkDeviceSize> > emptyRanges; //<offset, size>
};

class VMemoryAllocator : public Singleton<VMemoryAllocator>
{
    public:
        friend class Singleton<VMemoryAllocator>;
        friend class VApp;

        static bool allocMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, VMemory &vmemory);
        static bool freeMemory(VMemory &vmemory);

    protected:
        VMemoryAllocator();
        virtual ~VMemoryAllocator();

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        bool allocMemoryImpl(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, VMemory &vmemory);
        bool freeMemoryImpl(VMemory &vmemory);

        bool searchForSpace(AllocatedMemory *allocatedMemory, VkMemoryRequirements memRequirements, VkDeviceSize &offset);
        bool createMemory(uint32_t memoryTypeIndex);

        AllocatedMemory *findAllocatingMemory(VMemory &vmemory);

        void cleanAll();

    private:
        std::map<uint32_t, std::vector<AllocatedMemory*> > m_memories;

    public:
        static uint32_t MEMORY_SIZE;
};


}

#endif // VMEMORYALLOCATOR_H
