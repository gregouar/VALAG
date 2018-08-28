#include "Valag/gfx/SpritesBatch.h"

#include "Valag/core/VApp.h"

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
    if(m_sprites.insert(sprite).second)
        for(auto b : m_needToUpdateDrawCMB) b = 0;
}

bool SpritesBatch::removeSprite(Sprite *sprite)
{
    return m_sprites.erase(sprite);
}

VkCommandBuffer SpritesBatch::getDrawCommandBuffer(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    for(auto sprite : m_sprites)
    {
        if(sprite->checkUpdates(renderer, currentFrame))
            m_needToUpdateDrawCMB[currentFrame] = true;
    }

    return Drawable::getDrawCommandBuffer(renderer, currentFrame, renderPass, subpass, framebuffer);
}


bool SpritesBatch::recordDrawCommandBuffers(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass,
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

    if (vkBeginCommandBuffer(m_drawCommandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer");

    renderer->bindDefaultPipeline(m_drawCommandBuffers[currentFrame]);

    for(auto sprite : m_sprites)
        sprite->recordDrawCMBContent(m_drawCommandBuffers[currentFrame], renderer, currentFrame, renderPass, subpass, framebuffer);

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

    if (vkEndCommandBuffer(m_drawCommandBuffers[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");

    m_needToUpdateDrawCMB[currentFrame] = false;

    return (true);
}


void SpritesBatch::notify(NotificationSender* sender, NotificationType notificationType)
{
    /// Could check if sender is in m_sprites... or just trust it ?///
    if(notificationType == Notification_UpdateCMB)
        for(auto b : m_needToUpdateDrawCMB) b = true;
}


}
