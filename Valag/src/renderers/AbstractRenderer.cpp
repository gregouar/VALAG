#include "Valag/renderers/AbstractRenderer.h"

#include <array>

#include "Valag/utils/Profiler.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

AbstractRenderer::AbstractRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) :
    m_targetWindow(targetWindow),
    m_renderPass(VK_NULL_HANDLE),
    m_pipelineLayout(VK_NULL_HANDLE),
    m_pipeline(VK_NULL_HANDLE),
    m_descriptorPool(VK_NULL_HANDLE),
    m_curFrameIndex(0),
    m_order(order),
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

bool AbstractRenderer::createRenderPass()
{
    VkDevice device = VInstance::device();

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_targetWindow->getSwapchainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if(m_order == Renderer_First || m_order == Renderer_Unique)
    {
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    } else {
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if(m_order == Renderer_Last || m_order == Renderer_Unique)
    {
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } else {
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    return (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) == VK_SUCCESS);
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
