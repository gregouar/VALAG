#include "Valag/renderers/RenderView.h"

namespace vlg
{

RenderView::RenderView() :
    m_position(0.0f, 0.0f, 0.0f),
    m_lookAt(0.0f, 0.0f, -1.0f),
    m_zoom(1.0f),
    m_descriptorPool(VK_NULL_HANDLE),
    m_descriptorSetLayout(VK_NULL_HANDLE)
{
    //ctor
    m_viewUbo = {};
    m_viewUbo.depthOffsetAndFactor  = glm::vec2(0.0f, 0.0f);
    m_viewUbo.screenOffset          = glm::vec2(0.0f, 0.0f);
    m_viewUbo.screenSizeFactor      = glm::vec2(1.0f, 1.0f);
    m_viewUbo.view                  = glm::mat4(1.0f);
    m_viewUbo.viewInv               = glm::mat4(1.0f);
}

RenderView::~RenderView()
{
    this->destroy();
}

bool RenderView::create(size_t framesCount)
{
    if(!this->createBuffers(framesCount))
        return (false);
    if(!this->createDescriptorSetLayout())
        return (false);
    if(!this->createDescriptorPool(framesCount))
        return (false);
    if(!this->createDescriptorSets(framesCount))
        return (false);

    return (true);
}

void RenderView::destroy()
{
    VkDevice device = VInstance::device();

    for(auto buffer : m_buffers)
        VBuffersAllocator::freeBuffer(buffer);

    if(m_descriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
    m_descriptorSetLayout = VK_NULL_HANDLE;

    if(m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
    m_descriptorPool = VK_NULL_HANDLE;
}

void RenderView::update(size_t frameIndex)
{
    if(m_needToUpdateBuffers[frameIndex])
    {
        VBuffersAllocator::writeBuffer(m_buffers[frameIndex],&m_viewUbo,sizeof(m_viewUbo));
        m_needToUpdateBuffers[frameIndex] = false;
    }
}

void RenderView::setDepthFactor(float depthFactor)
{
    if(m_viewUbo.depthOffsetAndFactor.y != 1.0f/depthFactor)
        for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.depthOffsetAndFactor.y = 1.0f/depthFactor;
}

void RenderView::setExtent(glm::vec2 extent)
{
    if(m_viewUbo.screenSizeFactor.x != 2.0f/extent.x || m_viewUbo.screenSizeFactor.y != 2.0f/extent.y)
        for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.screenSizeFactor.x = 2.0f/extent.x;
    m_viewUbo.screenSizeFactor.y = 2.0f/extent.y;
}

void RenderView::setScreenOffset(glm::vec3 offset)
{
    if(m_viewUbo.screenOffset.x != offset.x || m_viewUbo.screenOffset.y != offset.y || m_viewUbo.depthOffsetAndFactor.x != offset.z)
        for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.screenOffset.x = offset.x;
    m_viewUbo.screenOffset.y = offset.y;
    m_viewUbo.depthOffsetAndFactor.x = offset.z;
}

void RenderView::setLookAt(glm::vec3 position, glm::vec3 lookAt)
{
    for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.view = glm::lookAt(position, lookAt, glm::vec3(0.0,0.0,1.0));
    m_viewUbo.viewInv = glm::inverse(m_viewUbo.view); ///Could maybe be optimize
}

void RenderView::setView(glm::mat4 view, glm::mat4 viewInv)
{
    for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.view      = view;
    m_viewUbo.viewInv   = viewInv;
}

void RenderView::setZoom(float zoom)
{

}


VkDescriptorSetLayout RenderView::getDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}

VkDescriptorSet RenderView::getDescriptorSet(size_t frameIndex)
{
    return m_descriptorSets[frameIndex];
}

bool RenderView::createBuffers(size_t framesCount)
{
    VkDeviceSize bufferSize = sizeof(ViewUBO);

    m_buffers.resize(framesCount);
    m_needToUpdateBuffers.resize(framesCount);

    for (size_t i = 0 ; i < framesCount ; ++i)
    {
        if(!VBuffersAllocator::allocBuffer(bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           m_buffers[i]))
            return (false);

        m_needToUpdateBuffers[i] = true;
    }
    return (true);
}

bool RenderView::createDescriptorSetLayout()
{
    VkDevice device = VInstance::device();

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; ///I could need to update to frag stage also
    layoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layoutBinding;

    return (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout) == VK_SUCCESS);
}

bool RenderView::createDescriptorPool(size_t framesCount)
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

bool RenderView::createDescriptorSets(size_t framesCount)
{
    VkDevice device = VInstance::device();

    std::vector<VkDescriptorSetLayout> layouts(framesCount, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(framesCount);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(framesCount);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_descriptorSets.data()) != VK_SUCCESS)
        return (false);

    for (size_t i = 0; i < framesCount ; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_buffers[i].buffer;
        bufferInfo.offset = m_buffers[i].offset;
        bufferInfo.range = sizeof(ViewUBO);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    return (true);
}



}
