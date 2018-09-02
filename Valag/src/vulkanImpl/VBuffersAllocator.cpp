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
    //vbuffer.bufferID = m_currentID++;
    vbuffer.bufferMemory = allocatedBuffer->bufferMemory;
   // vbuffer.memoryTypeIndex = allocatedBuffer->memoryTypeIndex;
    vbuffer.offset = offset;

    //std::cout<<vbuffer.offset<<" "<<vbuffer.alignedSize<<std::endl;

    return (true);
}

void VBuffersAllocator::writeBuffer(VBuffer dstBuffer, void* data, VkDeviceSize size)
{
    VBuffersAllocator::instance()->writeBufferImpl(dstBuffer, data, size);
}

void VBuffersAllocator::writeBufferImpl(VBuffer dstBuffer, void* data, VkDeviceSize size)
{
    auto device = VInstance::device();

    void* dstData;

    AllocatedBuffer *allBuffer = this->findAllocatingBuffer(dstBuffer);

    allBuffer->mutex.lock();
    vkMapMemory(device, dstBuffer.bufferMemory, dstBuffer.offset, size, 0, &dstData);
        memcpy(dstData, data, (size_t) size);
    vkUnmapMemory(device, dstBuffer.bufferMemory);
    allBuffer->mutex.unlock();
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
    //VBuffersAllocator::instance()->copyBufferImpl(srcBuffer, dstBuffer, size, commandPoolName);
}

/*void VBuffersAllocator::copyBufferImpl(VBuffer srcBuffer, VBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName)
{
    VkCommandBuffer commandBuffer = VInstance::instance()->beginSingleTimeCommands(commandPoolName);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = srcBuffer.offset;
    copyRegion.size = size;
    copyRegion.dstOffset = dstBuffer.offset;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    VInstance::instance()->endSingleTimeCommands(commandBuffer);
}*/


void VBuffersAllocator::copyBufferToImage(VBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer,
                                   CommandPoolName cmdPoolName)
{
   // VBuffersAllocator::instance()->copyBufferToImageImpl(buffer, image, width, height, layer, cmdPoolName);
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

/*void VBuffersAllocator::copyBufferToImageImpl(VBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer,
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
}*/

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

    //std::cout<<"First offset = "<<allocatedBuffer->emptyRanges.front().first<<std::endl;


    //for(auto& it : allocatedBuffer->emptyRanges)
    for(auto it = allocatedBuffer->emptyRanges.begin() ; it != allocatedBuffer->emptyRanges.end() ; ++it)
    {
        //std::cout<<it.first<<std::endl;
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
                /*
                for(auto &it2 : allocatedBuffer->emptyRanges)
                        std::cout<<it2.first<<" "<<it2.second<<std::endl;
                 std::cout<<std::endl;*/

                lastMerge->second += it->second;
                allocatedBuffer->emptyRanges.erase(it);

                /*for(auto &it2 : allocatedBuffer->emptyRanges)
                        std::cout<<it2.first<<" "<<it2.second<<std::endl;
                 std::cout<<std::endl<<std::endl;*/
            } else {
                it->first -= vbuffer.alignedSize;
                it->second += vbuffer.alignedSize;
                alreadyMerged = true;
            }
            break;
        }
    }

    if(!alreadyMerged)
    {

    /*std::cout<<"Will remove :"<<vbuffer.offset<<" "<<vbuffer.alignedSize<<" from :"<<std::endl;

    for(auto &it : allocatedBuffer->emptyRanges)
            std::cout<<it.first<<" "<<it.second<<std::endl;
     std::cout<<std::endl; */
        allocatedBuffer->emptyRanges.insert(leftmostRight,{vbuffer.offset, vbuffer.alignedSize});


/*    std::cout<<"We end up with :"<<std::endl;

    for(auto &it : allocatedBuffer->emptyRanges)
            std::cout<<it.first<<" "<<it.second<<std::endl;
     std::cout<<std::endl<<std::endl;*/
    }

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
    VkDevice device = VInstance::device();

    VkBuffer        buffer;
    VkDeviceMemory  bufferMemory;

    VulkanHelpers::createBuffer(BUFFER_SIZE,usage,properties,buffer,bufferMemory);

   /* VkBufferCreateInfo bufferInfo = {};
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

    vkBindBufferMemory(device, buffer, bufferMemory, 0);*/

    AllocatedBuffer *allocatedBuffer = new AllocatedBuffer();
    allocatedBuffer->buffer = buffer;
    allocatedBuffer->bufferMemory = bufferMemory;
    allocatedBuffer->bufferSize = BUFFER_SIZE;/*memRequirements.size;*/
    //allocatedBuffer->memoryTypeIndex = allocInfo.memoryTypeIndex;
    allocatedBuffer->emptyRanges.push_back({0, BUFFER_SIZE/*memRequirements.size*/});

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

    /*for(auto &subBuffer : m_allocatedSubBuffers)
        this->freeBufferImpl(subBuffer.second);
    m_allocatedSubBuffers.clear();*/

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
