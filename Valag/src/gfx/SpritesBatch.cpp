#include "Valag/gfx/SpritesBatch.h"

#include "Valag/core/VApp.h"
#include "Valag/utils/Profiler.h"

namespace vlg
{

SpritesBatch::SpritesBatch()
{
}

SpritesBatch::~SpritesBatch()
{
}

void SpritesBatch::addSprite(Sprite *sprite)
{
    //if(m_sprites.insert(sprite).second)
   // m_sprites.push_back(sprite);
    if(m_sortedSprites[sprite->getTexture()].insert(sprite).second)
    {
        sprite->askForAllNotifications(this);
        for(auto b : m_needToUpdateDrawCMB) b = 0;
    }
}

bool SpritesBatch::removeSprite(Sprite *sprite)
{

    //return m_sprites.erase(sprite);
    //m_sprites.remove(sprite);
    return m_sortedSprites[sprite->getTexture()].erase(sprite);
}

VkCommandBuffer SpritesBatch::getDrawCommandBuffer(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    for(auto spriteSet : m_sortedSprites)
    for(auto sprite : spriteSet.second)
    {
        if(sprite->checkUpdates(renderer, frameIndex))
            m_needToUpdateDrawCMB[frameIndex] = true;
    }

    return Drawable::getDrawCommandBuffer(renderer, frameIndex, renderPass, subpass, framebuffer);
}


bool SpritesBatch::recordDrawCommandBuffers(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                      uint32_t subpass, VkFramebuffer framebuffer)
{
    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = renderPass;
    inheritanceInfo.subpass = subpass;
    inheritanceInfo.framebuffer = framebuffer;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags = 0;
    inheritanceInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional

    if (vkBeginCommandBuffer(m_drawCommandBuffers[frameIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer");

    renderer->bindDefaultPipeline(m_drawCommandBuffers[frameIndex], frameIndex);

    for(auto spriteSet : m_sortedSprites)
    for(auto sprite : spriteSet.second)
        sprite->recordDrawCMBContent(m_drawCommandBuffers[frameIndex], renderer, frameIndex, renderPass, subpass, framebuffer);

    /*VkBuffer vertexBuffers[] = {m_vertexBuffer.buffer};
    VkDeviceSize offsets[] = {m_vertexBuffer.offset};

    if(!renderer->bindTexture(m_drawCommandBuffers[currentFrame], m_texture, currentFrame))
    {
        if (vkEndCommandBuffer(m_drawCommandBuffers[currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");
        return (false);
    }

    renderer->bindAllUBOs(m_drawCommandBuffers[currentFrame],currentFrame,m_modelUBOIndex);

    vkCmdBindVertexBuffers(m_drawCommandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

    vkCmdDraw(m_drawCommandBuffers[currentFrame], 4, 1, 0, 0);*/

    if (vkEndCommandBuffer(m_drawCommandBuffers[frameIndex]) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");

    m_needToUpdateDrawCMB[frameIndex] = false;

    return (true);
}


void SpritesBatch::notify(NotificationSender* sender, NotificationType notificationType)
{
    /// Could check if sender is in m_sprites... or just trust it ?///
    if(notificationType == Notification_UpdateCMB)
        for(auto b : m_needToUpdateDrawCMB) b = true;
    else if(notificationType == Notification_TextureIsAboutToChange)
    {
        Sprite *sprite = dynamic_cast<Sprite*>(sender);
        m_sortedSprites[sprite->getTexture()].erase(sprite);
    }
    else if(notificationType == Notification_TextureChanged)
    {
        Sprite *sprite = dynamic_cast<Sprite*>(sender);
        m_sortedSprites[sprite->getTexture()].insert(sprite);
    }
}


}
