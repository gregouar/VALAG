#include "Valag/vulkanImpl/RenderWindow.h"

#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/core/VApp.h"
#include "Valag/utils/Logger.h"
#include "Valag/utils/Parser.h"
#include "Valag/gfx/AbstractRenderer.h"


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


bool RenderWindow::create(size_t w, size_t h, const std::string &name, bool fullscreen)
{
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

    if(!this->createDepthStencil())
    {
        Logger::error("Cannot create depth stencil");
        return (false);
    }

    if(!this->createImageViews())
    {
        Logger::error("Cannot create image views");
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

size_t RenderWindow::getCurrentFrameIndex()
{
    return m_curFrameIndex;
}

VkExtent2D RenderWindow::getSwapchainExtent()
{
    return m_swapchainExtent;
}

VkFormat RenderWindow::getSwapchainImageFormat()
{
    return m_swapchainImageFormat;
}

const std::vector<VkImageView> &RenderWindow::getSwapchainImageViews()
{
    return m_swapchainImageViews;
}

const std::vector<VkImageView> &RenderWindow::getDepthStencilImageViews()
{
    return m_depthStencilImagesViews;
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

    m_finishedRenderingSemaphores[m_curFrameIndex].clear();

    m_curImageIndex = imageIndex;

    for(auto renderer : m_attachedRenderers)
        renderer.second->update(m_curImageIndex);

    return imageIndex;
}

void RenderWindow::submitToGraphicsQueue(VkCommandBuffer commandBuffer, VkSemaphore finishedRenderingSemaphore)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore[m_curFrameIndex]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {finishedRenderingSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VInstance::submitToGraphicsQueue(submitInfo, m_inFlightFences[m_curFrameIndex]);

    m_finishedRenderingSemaphores[m_curFrameIndex].push_back(finishedRenderingSemaphore);
}

void RenderWindow::display()
{
   // Profiler::pushClock("Update buffers");
    for(auto renderer : m_attachedRenderers)
        renderer.second->updateBuffers(m_curImageIndex);
   // Profiler::popClock();

    //Profiler::pushClock("Submit to queue");
    for(auto renderer : m_attachedRenderers)
        this->submitToGraphicsQueue(renderer.second->getCommandBuffer(m_curFrameIndex),
                                    renderer.second->getRenderFinishedSemaphore(m_curFrameIndex));
   // Profiler::popClock();

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(m_finishedRenderingSemaphores[m_curFrameIndex].size());
    presentInfo.pWaitSemaphores = m_finishedRenderingSemaphores[m_curFrameIndex].data();

    VkSwapchainKHR swapchains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &m_curImageIndex;

    presentInfo.pResults = nullptr;

    VInstance::presentQueue(presentInfo);

    m_curFrameIndex = (m_curFrameIndex + 1) % VApp::MAX_FRAMES_IN_FLIGHT;
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
    m_swapchainExtent = chooseSwapExtent(swapchainSupport.capabilities);

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
    createInfo.imageExtent = m_swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; /// Use VK_IMAGE_USAGE_TRANSFER_DST_BIT  for postfx

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
    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, NULL);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_swapchainImages.data());

    m_swapchainImageFormat = surfaceFormat.format;

    return (true);
}

bool RenderWindow::createDepthStencil()
{
    m_depthStencilImages.resize(m_swapchainImages.size());
    m_depthStencilImagesMemory.resize(m_swapchainImages.size());

    //VulkanHelpers::createImage();
    for (size_t i = 0; i < m_swapchainImages.size(); i++)
        if(!VulkanHelpers::createImage(m_swapchainExtent.width, m_swapchainExtent.height, 1, VK_FORMAT_D24_UNORM_S8_UINT,
                                    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    m_depthStencilImages[i], m_depthStencilImagesMemory[i]))
            return (false);
       // depthImagesView[i] = createImageView(depthImages[i], depthFormat);

    return (true);
}

bool RenderWindow::createImageViews()
{
    m_swapchainImageViews.resize(m_swapchainImages.size());
    m_depthStencilImagesViews.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImages.size(); ++i)
    {
        m_swapchainImageViews[i] =
            VulkanHelpers::createImageView(m_swapchainImages[i],m_swapchainImageFormat,VK_IMAGE_ASPECT_COLOR_BIT,1);
        m_depthStencilImagesViews[i] =
            VulkanHelpers::createImageView(m_depthStencilImages[i], VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

        VulkanHelpers::transitionImageLayout(m_depthStencilImages[i],0, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED,
                                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        /*VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;,
        createInfo.format = m_swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(VInstance::device(), &createInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS)
            return (false);*/
    }

    return (true);
}

bool RenderWindow::createSemaphoresAndFences()
{
    VkDevice device = VInstance::device();

    m_imageAvailableSemaphore.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_finishedRenderingSemaphores.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore[i]) != VK_SUCCESS)
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

    for (auto imageView : m_depthStencilImagesViews)
        vkDestroyImageView(device, imageView, nullptr);
    m_depthStencilImagesViews.clear();

    for(auto image : m_depthStencilImages)
        vkDestroyImage(device, image, nullptr);
    m_depthStencilImages.clear();

    for(auto memory : m_depthStencilImagesMemory)
        vkFreeMemory(device, memory, nullptr);
    m_depthStencilImagesMemory.clear();

    for (auto imageView : m_swapchainImageViews)
        vkDestroyImageView(device, imageView, nullptr);
    m_swapchainImageViews.clear();

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
