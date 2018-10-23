#include "Valag/gfx/Drawable.h"

#include "Valag/gfx/Sprite.h"

namespace vlg
{

Drawable::Drawable()
{
    m_needToCreateDrawCmb = true;
}

Drawable::~Drawable()
{
    //dtor
}

VkCommandBuffer Drawable::getDrawCommandBuffer(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    if(m_needToCreateDrawCmb)
        this->createDrawCommandBuffers();

    /*if(m_needToUpdateDrawCmb[frameIndex])
    {
        if(!this->recordDrawCommandBuffers(renderer, frameIndex, renderPass, subpass, framebuffer))
            return VK_NULL_HANDLE;
    }*/

    if(!this->recordDrawCommandBuffers(renderer, frameIndex, renderPass, subpass, framebuffer))
        return VK_NULL_HANDLE;

    return m_drawCommandBuffers[frameIndex];
}

void Drawable::createDrawCommandBuffers()
{
    m_drawCommandBuffers.resize(Sprite::s_framesCount);
    m_needToUpdateDrawCmb = std::vector<bool> (Sprite::s_framesCount, true);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VInstance::commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = (uint32_t) m_drawCommandBuffers.size();

    if (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_drawCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");

    for(auto b : m_needToUpdateDrawCmb) b = true;
    m_needToCreateDrawCmb = false;
}


bool Drawable::updateDrawCmb(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                        uint32_t subpass, VkFramebuffer framebuffer)
{
    if(! m_needToUpdateDrawCmb[frameIndex] )
        return (true);

    bool suc = this->recordDrawCommandBuffers(renderer, frameIndex, renderPass, subpass, framebuffer);
    if(suc)
        m_needToUpdateDrawCmb[frameIndex] = false;
    return suc;
}

void Drawable::askToUpdateDrawCmb(size_t frameIndex)
{
    if(!m_needToUpdateDrawCmb.empty())
        m_needToUpdateDrawCmb[frameIndex] = true;
}

void Drawable::notify(NotificationSender* sender, NotificationType notificationType,
                      size_t dataSize, char* data)
{
    if(notificationType == Notification_UpdateCmb)
        for(auto b : m_needToUpdateDrawCmb) b = true;
}



}

