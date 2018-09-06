#include "Valag/gfx/Drawable.h"

#include "Valag/core/VApp.h"

namespace vlg
{

Drawable::Drawable()
{
    m_needToCreateDrawCMB = true;
    m_needToUpdateDrawCMB = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, true);
}

Drawable::~Drawable()
{
    //dtor
}

VkCommandBuffer Drawable::getDrawCommandBuffer(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    if(m_needToCreateDrawCMB)
        this->createDrawCommandBuffers();

    if(m_needToUpdateDrawCMB[frameIndex])
    {
        if(!this->recordDrawCommandBuffers(renderer, frameIndex, renderPass, subpass, framebuffer))
            return VK_NULL_HANDLE;
    }

    return m_drawCommandBuffers[frameIndex];
}

void Drawable::createDrawCommandBuffers()
{
    m_drawCommandBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VInstance::commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = (uint32_t) m_drawCommandBuffers.size();

    if (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_drawCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");

    for(auto b : m_needToUpdateDrawCMB) b = true;
    m_needToCreateDrawCMB = false;
}





}

