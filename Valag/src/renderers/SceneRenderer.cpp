#include "Valag/renderers/SceneRenderer.h"

#include "Valag/scene/IsoSpriteEntity.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

SceneRenderer::SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) : AbstractRenderer(targetWindow, name, order)
{
    this->init();
}

SceneRenderer::~SceneRenderer()
{
    this->cleanup();
}

void SceneRenderer::update(size_t frameIndex)
{
    IsoSpriteEntity::updateRendering(frameIndex);
    AbstractRenderer::update(frameIndex);
}

bool SceneRenderer::recordPrimaryCommandBuffer(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;

    if (vkBeginCommandBuffer(m_primaryCMB[m_curFrameIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (false);
    }

    /*VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_targetWindow->getSwapchainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {0.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_primaryCMB[m_curFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        if(!m_activeSecondaryCommandBuffers.empty())
            vkCmdExecuteCommands(m_primaryCMB[m_curFrameIndex], (uint32_t) m_activeSecondaryCommandBuffers.size(), m_activeSecondaryCommandBuffers.data());

    vkCmdEndRenderPass(m_primaryCMB[m_curFrameIndex]);

    m_activeSecondaryCommandBuffers.clear();*/

    if (vkEndCommandBuffer(m_primaryCMB[m_curFrameIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}

bool SceneRenderer::init()
{
     IsoSpriteEntity::initRendering(m_targetWindow->getFramesCount());

     return AbstractRenderer::init();
}

bool SceneRenderer::createRenderPass()
{
    return AbstractRenderer::createRenderPass();
}

bool SceneRenderer::createDescriptorSetLayouts()
{
    return (true);
}

bool SceneRenderer::createGraphicsPipeline()
{
    return (true);
}

bool SceneRenderer::createDescriptorPool()
{
    return (true);
}

bool SceneRenderer::createDescriptorSets()
{
    return (true);
}

bool SceneRenderer::createUBO()
{
    return (true);
}

void SceneRenderer::cleanup()
{
    AbstractRenderer::cleanup();
    IsoSpriteEntity::cleanupRendering();
}


}
