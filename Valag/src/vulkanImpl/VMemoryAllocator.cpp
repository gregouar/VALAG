#include "Valag/vulkanImpl/VMemoryAllocator.h"

#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/utils/Logger.h"

namespace vlg
{


uint32_t VMemoryAllocator::MEMORY_SIZE = 256*1024*1024; //256 mb

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

bool VMemoryAllocator::allocMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, VMemory &vmemory)
{
    return VMemoryAllocator::instance()->allocMemoryImpl(memRequirements, properties, vmemory);
}

bool VMemoryAllocator::allocMemoryImpl(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, VMemory &vmemory)
{
    uint32_t memoryTypeIndex = this->findMemoryType(memRequirements.memoryTypeBits, properties);

    auto itAllocatedMemories = m_memories.find(memoryTypeIndex);

    if(itAllocatedMemories == m_memories.end())
    {
        this->createMemory(memoryTypeIndex);
        return this->allocMemoryImpl(memRequirements, properties, vmemory);
    }

    VkDeviceSize offset = 0;
    AllocatedMemory *allocatedMemory = nullptr;

    vmemory.memRequirements = memRequirements;

    size_t i = 0;
    for(auto it : itAllocatedMemories->second)
    {
        if(this->searchForSpace(it,memRequirements,offset) == true)
        {
            allocatedMemory = it;
            vmemory.allocatingMemory = i;

            break;
        }
        ++i;
    }

    if(allocatedMemory == nullptr)
    {
        this->createMemory(memoryTypeIndex);
        return this->allocMemoryImpl(memRequirements,properties,vmemory);
    }

    vmemory.memoryTypeIndex = memoryTypeIndex;
    vmemory.vkMemory = allocatedMemory->vkMemory;
    vmemory.offset = offset;

   // std::cout<<memoryTypeIndex<<"  "<<memRequirements.size<<" "<<offset<<" "<<memRequirements.alignment<<std::endl;

    return (true);
}

bool VMemoryAllocator::freeMemory(VMemory &vmemory)
{
    return VMemoryAllocator::instance()->freeMemoryImpl(vmemory);
}


bool VMemoryAllocator::freeMemoryImpl(VMemory &vmemory)
{
    AllocatedMemory *allocatedMemory = this->findAllocatingMemory(vmemory);

    if(allocatedMemory == nullptr)
        return (false);

    bool alreadyMerged = false;
    auto lastMerge = allocatedMemory->emptyRanges.begin();

    auto leftmostRight = allocatedMemory->emptyRanges.begin();

    for(auto it = allocatedMemory->emptyRanges.begin() ; it != allocatedMemory->emptyRanges.end() ; ++it)
    {
        if(it->first < vmemory.offset)
            ++leftmostRight;


        if(it->first+it->second == vmemory.offset)
        {
            it->second += vmemory.memRequirements.size;
            alreadyMerged = true;
            lastMerge = it;
        }
        else if(it->first == vmemory.offset+vmemory.memRequirements.size)
        {
            if(alreadyMerged)
            {
                lastMerge->second += it->second;
                allocatedMemory->emptyRanges.erase(it);
            } else {
                it->first -= vmemory.memRequirements.size;
                it->second += vmemory.memRequirements.size;
                alreadyMerged = true;
            }
            break;
        }
    }

    if(!alreadyMerged)
        allocatedMemory->emptyRanges.insert(leftmostRight,{vmemory.offset, vmemory.memRequirements.size});

    vmemory.vkMemory = VK_NULL_HANDLE;

    return (true);
}

bool VMemoryAllocator::searchForSpace(AllocatedMemory *allocatedMemory, VkMemoryRequirements memRequirements, VkDeviceSize &offset)
{
    //for(auto &emptyRange : allocatedMemory->emptyRanges)
    for(auto emptyRange = allocatedMemory->emptyRanges.begin() ; emptyRange != allocatedMemory->emptyRanges.end() ; ++emptyRange)
    {
        VkDeviceSize oldEmptyRangeOffset = emptyRange->first;
        VkDeviceSize alignedEmptyRange = emptyRange->first;
        VkDeviceSize mod = alignedEmptyRange % memRequirements.alignment;
        VkDeviceSize increasedSize = 0;
        if(mod != 0)
            increasedSize = memRequirements.alignment - mod;

        alignedEmptyRange += increasedSize;

        if(emptyRange->second >= memRequirements.size + increasedSize)
        {
            offset = alignedEmptyRange;

            emptyRange->first += memRequirements.size + increasedSize;
            emptyRange->second -= memRequirements.size + increasedSize;

            if(increasedSize != 0)
            {
                ///I could not add the emptyRange but then I need to keep track of it in vmemory
                allocatedMemory->emptyRanges.insert(emptyRange,{oldEmptyRangeOffset, increasedSize});
            }

            if(emptyRange->second == 0)
                allocatedMemory->emptyRanges.erase(emptyRange);

            return (true);
        }
    }

    return (false);
}

bool VMemoryAllocator::createMemory(uint32_t memoryTypeIndex)
{
    VkDeviceMemory  memory;

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = MEMORY_SIZE;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(VInstance::device(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        Logger::error("Failed to allocate memory");
        return (false);
    }

    AllocatedMemory *allocatedMemory = new AllocatedMemory();
    allocatedMemory->vkMemory = memory;
    allocatedMemory->memorySize = MEMORY_SIZE;
    allocatedMemory->emptyRanges.push_back({0, MEMORY_SIZE});

    m_memories[memoryTypeIndex].push_back(allocatedMemory);

    return (true);
}

AllocatedMemory *VMemoryAllocator::findAllocatingMemory(VMemory &vmemory)
{
    auto it = m_memories.find(vmemory.memoryTypeIndex);
    if(it == m_memories.end())
        return nullptr;

    return it->second[vmemory.allocatingMemory];
}

void VMemoryAllocator::cleanAll()
{
    VkDevice device = VInstance::device();

    for(auto memoryList : m_memories)
    for(auto memory : memoryList.second)
    {
        vkFreeMemory(device, memory->vkMemory, nullptr);
        delete memory;
    }
    m_memories.clear();
}

}

