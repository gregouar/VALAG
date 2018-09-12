#include "Valag/renderers/AbstractRenderer.h"

#include <array>

#include "Valag/utils/Profiler.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

AbstractRenderer::AbstractRenderer(RenderWindow *targetWindow, RendererName name) :
    m_targetWindow(targetWindow),
    m_renderPass(VK_NULL_HANDLE),
    m_pipelineLayout(VK_NULL_HANDLE),
    m_pipeline(VK_NULL_HANDLE),
    m_descriptorPool(VK_NULL_HANDLE),
    m_curFrameIndex(0),
    m_name(name)
{
}

AbstractRenderer::~AbstractRenderer()
{
}


void AbstractRenderer::update(size_t frameIndex)
{
    m_renderView.update(frameIndex);
}

void AbstractRenderer::updateCMB(uint32_t imageIndex)
{
    Profiler::pushClock("Record primary buffer");
    this->recordPrimaryCommandBuffer(imageIndex);
    Profiler::popClock();

    m_curFrameIndex = (m_curFrameIndex + 1) % m_targetWindow->getFramesCount();
}

VkCommandBuffer AbstractRenderer::getCommandBuffer(size_t frameIndex)
{
    return m_primaryCMB[frameIndex];
}

/*VkSemaphore AbstractRenderer::getRenderFinishedSemaphore(size_t frameIndex)
{
    return m_renderFinishedSemaphore[frameIndex];
}*/

RendererName AbstractRenderer::getName()
{
    return m_name;
}


bool AbstractRenderer::createSwapchainFramebuffers()
{
    if(m_renderPass == VK_NULL_HANDLE)
        return (true);

    auto swapChainImageViews = m_targetWindow->getSwapchainImageViews();
    auto depthImagesView = m_targetWindow->getDepthStencilImageViews();

    m_swapchainFramebuffers.resize(swapChainImageViews.size());
    m_swapchainExtents.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImagesView[i]
        };

        m_swapchainExtents[i] = m_targetWindow->getSwapchainExtent();

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapchainExtents[i].width;
        framebufferInfo.height = m_swapchainExtents[i].height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS)
            return (false);

    }

    return (true);
}

bool AbstractRenderer::createPrimaryCommandBuffers()
{
    m_primaryCMB.resize(m_targetWindow->getFramesCount());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VInstance::commandPool(/*COMMANDPOOL_DEFAULT*/COMMANDPOOL_SHORTLIVED);
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) m_primaryCMB.size();

    if (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_primaryCMB.data()) != VK_SUCCESS)
    {
        Logger::error("Failed to allocate command buffers");
        return (false);
    }

    return (true);
}

bool AbstractRenderer::createRenderView()
{
    return m_renderView.create(m_targetWindow->getFramesCount());
}

/*bool AbstractRenderer::createSemaphores()
{
    m_renderFinishedSemaphore.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for(size_t i = 0 ; i < m_renderFinishedSemaphore.size() ; ++i)
        if(vkCreateSemaphore(VInstance::device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore[i]) != VK_SUCCESS)
            return (false);

    return (true);
}*/

bool AbstractRenderer::init()
{
    if(!this->createRenderPass())
    {
        Logger::error("Cannot create default render pass");
        return (false);
    }

    if(!this->createDescriptorSetLayouts())
    {
        Logger::error("Cannot create default descriptor set layout");
        return (false);
    }

    if(!this->createRenderView())
    {
        Logger::error("Cannot create render view");
        return (false);
    }

    if(!this->createGraphicsPipeline())
    {
        Logger::error("Cannot create graphics pipeline");
        return (false);
    }

    if(!this->createSwapchainFramebuffers())
    {
        Logger::error("Cannot create swapchain framebuffers");
        return (false);
    }


    if(!this->createUBO())
    {
        Logger::error("Cannot create UBOs");
        return (false);
    }

    if(!this->createDescriptorPool())
    {
        Logger::error("Cannot create descriptor pool");
        return (false);
    }

    if(!this->createDescriptorSets())
    {
        Logger::error("Cannot create descriptor sets");
        return (false);
    }

    if(!this->createPrimaryCommandBuffers())
    {
        Logger::error("Cannot create primary command buffers");
        return (false);
    }

    /*if(!this->createSemaphores())
    {
        Logger::error("Cannot create default renderer semaphores");
        return (false);
    }*/

    return (true);
}

void AbstractRenderer::cleanup()
{
    VkDevice device = VInstance::device();

    m_renderView.destroy();

    if(m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device,m_descriptorPool,nullptr);
    m_descriptorPool = VK_NULL_HANDLE;

   // for (auto semaphore : m_renderFinishedSemaphore)
      //  vkDestroySemaphore(device, semaphore, nullptr);

    for (auto framebuffer : m_swapchainFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    if(m_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;

    if(m_pipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    m_pipelineLayout = VK_NULL_HANDLE;

    if(m_renderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(device, m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;
}


}
