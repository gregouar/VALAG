#include "Valag/renderers/RenderWindow.h"

#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/core/VApp.h"
#include "Valag/utils/Logger.h"
#include "Valag/utils/Parser.h"

#include "Valag/renderers/AbstractRenderer.h"


namespace vlg
{

RenderWindow::RenderWindow() :
    m_window(nullptr),
    m_surface(VK_NULL_HANDLE),
    m_swapchain(VK_NULL_HANDLE)
{
    //ctor
}

RenderWindow::~RenderWindow()
{
    this->destroy();
}


bool RenderWindow::create(size_t w, size_t h, const std::string &name, bool fullscreen, size_t framesCount)
{
    m_framesCount = framesCount;

    if(!this->createGLFWindow(w,h,name,fullscreen))
        return (false);

    if(!this->createSurface())
        return (false);

    return (true);
}

bool RenderWindow::init()
{
    m_curFrameIndex = 0;

    if(!this->createSwapchain())
    {
        Logger::error("Cannot create swapchain");
        return (false);
    }

    if(!this->createSemaphoresAndFences())
    {
        Logger::error("Cannot create semaphores and fences");
        return (false);
    }

    return (true);
}


bool RenderWindow::attachRenderer(AbstractRenderer *renderer)
{
    RendererName name = renderer->getName();

    auto foundedRenderer = m_attachedRenderers.find(name);
    if(foundedRenderer != m_attachedRenderers.end())
        return (false);

    m_attachedRenderers.insert(foundedRenderer,{name,renderer});
    return (true);
}

bool RenderWindow::detachRenderer(RendererName renderer)
{
    auto foundedRenderer = m_attachedRenderers.find(renderer);
    if(foundedRenderer != m_attachedRenderers.end())
    {
        m_attachedRenderers.erase(foundedRenderer);
        return (true);
    }
    return (false);
}

size_t RenderWindow::getFramesCount()
{
    return m_framesCount;
}

size_t RenderWindow::getSwapchainSize()
{
    return m_swapchainAttachments.size();
}

size_t RenderWindow::getFrameIndex()
{
    return m_curFrameIndex;
}

size_t RenderWindow::getImageIndex()
{
    return m_curImageIndex;
}

VkExtent2D RenderWindow::getSwapchainExtent()
{
    return m_swapchainAttachments[0].extent;
}

VkFormat RenderWindow::getSwapchainImageFormat()
{
    return m_swapchainAttachments[0].type.format;
}

const std::vector<VFramebufferAttachment> &RenderWindow::getSwapchainAttachments()
{
    return m_swapchainAttachments;
}

const std::vector<VFramebufferAttachment> &RenderWindow::getSwapchainDepthAttachments()
{
    return m_depthStencilAttachments;
}

AbstractRenderer *RenderWindow::getRenderer(RendererName renderer)
{
    auto foundedRenderer = m_attachedRenderers.find(renderer);
    if(foundedRenderer != m_attachedRenderers.end())
        return foundedRenderer->second;

    return (nullptr);
}


/// PROTECTED ///

uint32_t RenderWindow::acquireNextImage()
{
    VkDevice device = VInstance::device();

    vkWaitForFences(device, 1, &m_inFlightFences[m_curFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &m_inFlightFences[m_curFrameIndex]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, m_swapchain, std::numeric_limits<uint64_t>::max(),
                          m_imageAvailableSemaphore[m_curFrameIndex], VK_NULL_HANDLE, &imageIndex);

    m_curImageIndex = imageIndex;

    for(auto renderer : m_attachedRenderers)
        renderer.second->update(m_curFrameIndex/**m_curImageIndex**/);

    return imageIndex;
}

void RenderWindow::submitToGraphicsQueue(std::vector<VkCommandBuffer> &commandBuffers, std::vector<VkSemaphore> &waitSemaphores)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkPipelineStageFlags> waitStages(waitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    waitSemaphores.push_back(m_imageAvailableSemaphore[m_curFrameIndex]);
    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    submitInfo.waitSemaphoreCount = /*1;// */ static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = /*&waitSemaphores.back();// */ waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    submitInfo.pCommandBuffers = commandBuffers.data();

    VkSemaphore signalSemaphores[] = {m_finishedRenderingSemaphores[m_curFrameIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VInstance::submitToGraphicsQueue(submitInfo, m_inFlightFences[m_curFrameIndex]);
}

void RenderWindow::display()
{
   // Profiler::pushClock("Update buffers");
    for(auto renderer : m_attachedRenderers)
        renderer.second->render(m_curImageIndex);
   // Profiler::popClock();

    //Profiler::pushClock("Submit to queue");
    std::vector<VkCommandBuffer> cmb;
    std::vector<VkSemaphore> waitSemaphores;

    for(auto renderer : m_attachedRenderers)
    {
        auto finalPasses = renderer.second->getFinalPasses();

        for(auto pass : finalPasses)
        {
            ///Could add stages too
            cmb.push_back(*pass->getPrimaryCmb(m_curImageIndex, m_curFrameIndex));
            waitSemaphores.insert(waitSemaphores.end(),
                                  pass->getWaitSemaphores(m_curFrameIndex).begin(),
                                  pass->getWaitSemaphores(m_curFrameIndex).end());
        }
    }
    this->submitToGraphicsQueue(cmb,waitSemaphores);
   // Profiler::popClock();

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;//static_cast<uint32_t>(m_finishedRenderingSemaphores[m_curFrameIndex].size());
    presentInfo.pWaitSemaphores = &m_finishedRenderingSemaphores[m_curFrameIndex];//m_finishedRenderingSemaphores[m_curFrameIndex].data();

    VkSwapchainKHR swapchains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &m_curImageIndex;

    presentInfo.pResults = nullptr;

    VInstance::presentQueue(presentInfo);

    m_curFrameIndex = (m_curFrameIndex + 1) % m_framesCount;
}

bool RenderWindow::checkVideoMode(size_t w, size_t h, GLFWmonitor *monitor)
{
    int count;
    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

    bool ok = false;

    for(auto i = 0 ; i < count ; ++i)
    {
        if(modes[i].width == (int)w && modes[i].height == (int)h)
            ok = true;
    }

    return ok;
}

bool RenderWindow::createGLFWindow(size_t w, size_t h, const std::string &name, bool fullscreen)
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if(!this->checkVideoMode(w,h,monitor))
    {
        std::ostringstream error_report;
        error_report<<"Invalid resolution "<<w<<"x"<<h;
        Logger::error(error_report);

        w = Parser::parseInt(VApp::DEFAULT_WINDOW_WIDTH);
        h = Parser::parseInt(VApp::DEFAULT_WINDOW_HEIGHT);

        if(!this->checkVideoMode(w,h,monitor))
        {
            std::ostringstream error_report;
            error_report<<"Invalid default resolution "<<w<<"x"<<h;
            Logger::error(error_report);

            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            w = mode->width;
            h = mode->height;
        }
    }

    if(!fullscreen)
        monitor = nullptr;

    m_window = glfwCreateWindow(w,h,name.c_str(), monitor, nullptr);

    return (m_window != nullptr);
}

bool RenderWindow::createSurface()
{
    return (glfwCreateWindowSurface(VInstance::instance()->getVulkanInstance(), m_window, nullptr, &m_surface) == VK_SUCCESS);
}

VkSurfaceFormatKHR RenderWindow::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

     for (const auto& availableFormat : availableFormats)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
        && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;

    return availableFormats[0];
}

VkPresentModeKHR RenderWindow::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    /** There is something fishy here... **/

    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
        /*else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            bestMode = availablePresentMode;*/
    }

    return bestMode;
}

VkExtent2D RenderWindow::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int w, h;
    glfwGetWindowSize(m_window, &w, &h);
    VkExtent2D actualExtent = { static_cast<uint32_t>(w),
                                static_cast<uint32_t>(h)};

    actualExtent.width  = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

bool RenderWindow::createSwapchain()
{
    VkDevice device = VInstance::device();

    SwapchainSupportDetails swapchainSupport = VInstance::instance()->querySwapchainSupport(VInstance::physicalDevice(), m_surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D swapchainExtent = chooseSwapExtent(swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0
     && imageCount > swapchainSupport.capabilities.maxImageCount)
        imageCount = swapchainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT; /// Use VK_IMAGE_USAGE_TRANSFER_DST_BIT  for postfx ?

    QueueFamilyIndices queueFamilyIndices = VInstance::instance()->getQueueFamilyIndices();
    uint32_t queueFamilyIndicesArray[] = {(uint32_t) queueFamilyIndices.graphicsFamily,
                                     (uint32_t) queueFamilyIndices.presentFamily};

    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        return (false);

    imageCount = 0;
    std::vector<VkImage> tempSwapchainImages;
    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, NULL);
    tempSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, tempSwapchainImages.data());

    if(m_framesCount > imageCount)
        m_framesCount = imageCount;

    m_swapchainAttachments.resize(imageCount);
    m_depthStencilAttachments.resize(imageCount);

    for(size_t i = 0 ; i < tempSwapchainImages.size() ; ++i)
    {
        m_swapchainAttachments[i].extent = swapchainExtent;
        m_swapchainAttachments[i].type.format = surfaceFormat.format;
        m_swapchainAttachments[i].image.vkImage  = tempSwapchainImages[i];
        m_swapchainAttachments[i].image.memory  = {};
        m_swapchainAttachments[i].view =
                VulkanHelpers::createImageView(m_swapchainAttachments[i].image.vkImage,surfaceFormat.format,
                                               VK_IMAGE_ASPECT_COLOR_BIT,1);
        m_swapchainAttachments[i].type.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VulkanHelpers::createAttachment(swapchainExtent.width, swapchainExtent.height, VK_FORMAT_D24_UNORM_S8_UINT,
                                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, m_depthStencilAttachments[i]);
    }

    return (true);
}

bool RenderWindow::createSemaphoresAndFences()
{
    VkDevice device = VInstance::device();

    m_imageAvailableSemaphore.resize(m_framesCount);
    m_finishedRenderingSemaphores.resize(m_framesCount);
    m_inFlightFences.resize(m_framesCount);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0 ; i < m_framesCount ; ++i)
    {
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore[i]) != VK_SUCCESS)
            return (false);

        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_finishedRenderingSemaphores[i]) != VK_SUCCESS)
            return (false);

        if(vkCreateFence(device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            return (false);
    }

    return (true);
}

void RenderWindow::destroy()
{
    VkDevice device = VInstance::device();

    for (auto fence : m_inFlightFences)
        vkDestroyFence(device, fence, nullptr);
    m_inFlightFences.clear();

    for (auto semaphore : m_imageAvailableSemaphore)
        vkDestroySemaphore(device, semaphore, nullptr);
    m_imageAvailableSemaphore.clear();

    for (auto semaphore : m_finishedRenderingSemaphores)
        vkDestroySemaphore(device, semaphore, nullptr);
    m_finishedRenderingSemaphores.clear();

    for(auto attachment : m_depthStencilAttachments)
        VulkanHelpers::destroyAttachment(attachment);
    m_depthStencilAttachments.clear();

    for (auto attachment : m_swapchainAttachments)
        vkDestroyImageView(device, attachment.view, nullptr);
    m_swapchainAttachments.clear();

    if(m_swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(device, m_swapchain, nullptr);
    m_swapchain = VK_NULL_HANDLE;

    if(m_surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(VInstance::instance()->getVulkanInstance(), m_surface, nullptr);
    m_surface = VK_NULL_HANDLE;

    if(m_window != nullptr)
        glfwDestroyWindow(m_window);
    m_window = nullptr;
}

VkSurfaceKHR &RenderWindow::getSurface()
{
    return m_surface;
}


GLFWwindow *RenderWindow::getWindowPtr()
{
    return m_window;
}

}
