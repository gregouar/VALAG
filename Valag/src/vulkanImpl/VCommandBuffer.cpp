#include "Valag/vulkanImpl/VCommandBuffer.h"

#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/vulkanImpl/VulkanHelpers.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

VCommandBuffer::VCommandBuffer()
{
    //ctor
}

VCommandBuffer::~VCommandBuffer()
{
    //dtor
}

bool VCommandBuffer::init(size_t buffersCount)
{
    m_buffersCount  = buffersCount;

    return this->createBuffers();
}

void VCommandBuffer::setCmbUsage(VkFlags usage)
{
    m_cmbUsage = usage;
}

VkFlags VCommandBuffer::getCmbUsage()
{
    return m_cmbUsage;
}

const VkCommandBuffer *VCommandBuffer::getVkCommandBuffer(size_t cmbIndex)
{
    return &m_cmbs[cmbIndex];
}

VkCommandBuffer VCommandBuffer::startRecording(size_t bufferIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = m_cmbUsage;

    if (vkBeginCommandBuffer(m_cmbs[bufferIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (VK_NULL_HANDLE);
    }

    m_lastRecording = bufferIndex;
    m_inRenderPass  = false;

    return m_cmbs[bufferIndex];
}

VkCommandBuffer VCommandBuffer::startRecording(size_t bufferIndex, size_t framebufferIndex, VkSubpassContents contents,
                                                VRenderPass* renderPass, VRenderTarget *renderTarget)
{
    VkCommandBuffer cmb = this->startRecording(bufferIndex);

    if(cmb != VK_NULL_HANDLE)
        this->nextRenderPass(framebufferIndex,contents, renderPass, renderTarget);

    return cmb;
}

void VCommandBuffer::nextRenderPass(size_t framebufferIndex, VkSubpassContents contents,
                                    VRenderPass* renderPass, VRenderTarget *renderTarget)
{
    if(m_inRenderPass)
        vkCmdEndRenderPass(m_cmbs[m_lastRecording]);

    renderTarget->startRendering(framebufferIndex, m_cmbs[m_lastRecording], contents, renderPass);

    m_inRenderPass = true;
}

bool VCommandBuffer::endRecording()
{
    if(m_inRenderPass)
        vkCmdEndRenderPass(m_cmbs[m_lastRecording]);

    if (vkEndCommandBuffer(m_cmbs[m_lastRecording]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}

/// Protected ///

bool VCommandBuffer::createBuffers()
{
    m_cmbs.resize(m_buffersCount);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandPoolName pool    = (m_cmbUsage == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? COMMANDPOOL_SHORTLIVED : COMMANDPOOL_DEFAULT;
    allocInfo.commandPool   = VInstance::commandPool(pool);
    allocInfo.level         = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_cmbs.size());

    return (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_cmbs.data()) == VK_SUCCESS);
}

}
