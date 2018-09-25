#include "Valag/vulkanImpl/DynamicUBODescriptor.h"

namespace vlg
{

DynamicUBODescriptor::DynamicUBODescriptor(size_t objectSize, size_t chunkSize) :
    m_descriptorSetLayout(VK_NULL_HANDLE),
    m_descriptorPool(VK_NULL_HANDLE),
    m_objectSize(objectSize),
    m_chunkSize(chunkSize)
{
    //ctor
}

DynamicUBODescriptor::~DynamicUBODescriptor()
{
    //dtor
}

bool DynamicUBODescriptor::init(size_t framesCount)
{
    if(!this->createDescriptorSetLayouts())
        return (false);

    m_needToExpandBuffers = std::vector<bool> (framesCount, false);
    m_buffers.resize(framesCount);
    for(size_t i = 0 ; i < framesCount ; ++i)
        m_buffers[i] = new DynamicUBO(m_objectSize,m_chunkSize);

    if(!this->createDescriptorPool(framesCount))
        return (false);

    if(!this->createDescriptorSets(framesCount))
        return (false);

    return (true);
}

void DynamicUBODescriptor::update(size_t frameIndex)
{
    if(m_needToExpandBuffers[frameIndex])
    {
        m_buffers[frameIndex]->expandBuffers(/*false*/);
        this->updateDescriptorSets(frameIndex);
        m_needToExpandBuffers[frameIndex] = false;
    }
}

void DynamicUBODescriptor::cleanup()
{
    auto device = VInstance::device();

    if(m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device,m_descriptorPool,nullptr);
    m_descriptorPool = VK_NULL_HANDLE;

    for(auto buffer : m_buffers)
        delete buffer;
    m_buffers.clear();

    if(m_descriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
    m_descriptorPool = VK_NULL_HANDLE;
}

bool DynamicUBODescriptor::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if(vkCreateDescriptorSetLayout(VInstance::device(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
        return (false);

    return (true);
}

bool DynamicUBODescriptor::createDescriptorPool(size_t framesCount)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(framesCount);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(framesCount);

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool DynamicUBODescriptor::createDescriptorSets(size_t framesCount)
{
    std::vector<VkDescriptorSetLayout> layouts(framesCount, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(framesCount);
    if (vkAllocateDescriptorSets(VInstance::device(), &allocInfo,m_descriptorSets.data()) != VK_SUCCESS)
        return (false);

    for (size_t i = 0; i < m_descriptorSets.size() ; ++i)
        this->updateDescriptorSets(i);

    return (true);
}


void DynamicUBODescriptor::updateDescriptorSets(size_t frameIndex)
{
    VkDevice device = VInstance::device();

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_buffers[frameIndex]->getBuffer().buffer;
    bufferInfo.offset = m_buffers[frameIndex]->getBuffer().offset;
    bufferInfo.range = m_objectSize;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSets[frameIndex];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}



bool DynamicUBODescriptor::allocObject(size_t frameIndex, size_t &index)
{
    if(m_buffers[frameIndex]->isFull())
    {
        m_needToExpandBuffers[frameIndex] = true;
        return (false);
    }

    return m_buffers[frameIndex]->allocObject(index);
}

bool DynamicUBODescriptor::freeObject(size_t frameIndex, size_t index)
{
    if(m_buffers.empty())
        return (true);
    return m_buffers[frameIndex]->freeObject(index);
}

bool DynamicUBODescriptor::updateObject(size_t frameIndex, size_t index, void *data)
{
    return m_buffers[frameIndex]->updateObject(index, data);
}


size_t DynamicUBODescriptor::getBufferVersion(size_t frameIndex)
{
    return m_buffers[frameIndex]->getBufferVersion();
}

VkDescriptorSetLayout DynamicUBODescriptor::getDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}

VkDescriptorSet DynamicUBODescriptor::getDescriptorSet(size_t frameIndex)
{
    return m_descriptorSets[frameIndex];
}

uint32_t DynamicUBODescriptor::getDynamicOffset(size_t frameIndex, size_t index)
{
    return m_buffers[frameIndex]->getDynamicOffset(index);
}

/*VkDescriptorSetLayout DynamicUBODescriptor::getDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}*/

}

