#include "Valag/gfx/VInstance.h"

#include <cstring>
#include <map>
#include <set>
#include <glm/glm.hpp>

#include "Valag/VulkanExtProxies.h"

#include "Valag/utils/Logger.h"
#include "Valag/core/Config.h"
#include "Valag/core/VApp.h"

namespace vlg
{
const std::vector<const char*> VInstance::const_validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> VInstance::const_deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool const_enableValidationLayers = false;
#else
    const bool const_enableValidationLayers = true;
#endif

VInstance *VInstance::static_currentInstance = nullptr;

VInstance::VInstance(GLFWwindow *window, const std::string name) :
    m_name(name),
    m_parentWindow(window),
    m_physicalDevice(VK_NULL_HANDLE),
    m_isInit(false)
{
    this->init();
}

VInstance::~VInstance()
{
    this->cleanup();
}

uint32_t VInstance::acquireNextImage()
{
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_device, m_swapchain, std::numeric_limits<uint64_t>::max(),
                          m_imageAvailableSemaphore[m_currentFrame], VK_NULL_HANDLE, &imageIndex);


    m_finishedRenderingSemaphores[m_currentFrame].clear();

    m_curImageIndex = imageIndex;

    return imageIndex;
}

void VInstance::submitToGraphicsQueue(VkCommandBuffer commandBuffer, VkSemaphore finishedRenderingSemaphore)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {finishedRenderingSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;


    m_graphicsQueueAccessMutex.lock();
        if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer");
    m_graphicsQueueAccessMutex.unlock();

    m_finishedRenderingSemaphores[m_currentFrame].push_back(finishedRenderingSemaphore);
}

void VInstance::presentQueue()
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(m_finishedRenderingSemaphores[m_currentFrame].size());
    presentInfo.pWaitSemaphores = m_finishedRenderingSemaphores[m_currentFrame].data();

    VkSwapchainKHR swapchains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &m_curImageIndex;

    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(m_presentQueue, &presentInfo);

    m_currentFrame = (m_currentFrame + 1) % VApp::MAX_FRAMES_IN_FLIGHT;
}

void VInstance::waitDeviceIdle()
{
    vkDeviceWaitIdle(this->getDevice());
}

size_t VInstance::getCurrentFrameIndex()
{
    return m_currentFrame;
}

bool VInstance::isInitialized()
{
    return m_isInit;
}

void VInstance::setActive()
{
    VInstance::setCurrentInstance(this);
}

VkExtent2D VInstance::getSwapchainExtent()
{
    return m_swapchainExtent;
}


VkFormat VInstance::getSwapchainImageFormat()
{
    return m_swapchainImageFormat;
}

void VInstance::init()
{
    m_currentFrame = 0;

    if(!this->createVulkanInstance())
        throw std::runtime_error("Cannot create Vulkan instance");

    if(!this->setupDebugCallback())
        throw std::runtime_error("Cannot setup debug callback");

    if(!this->createSurface())
        throw std::runtime_error("Cannot create window surface");

    if(!this->pickPhysicalDevice())
        throw std::runtime_error("Cannot find suitable physical device");

    if(!this->createLogicalDevice())
        throw std::runtime_error("Cannot create logical device");

    if(!this->createSwapchain())
        throw std::runtime_error("Cannot create swapchain");

    if(!this->createImageViews())
        throw std::runtime_error("Cannot create image views");

    if(!this->createCommandPools())
        throw std::runtime_error("Cannot create command pools");

    if(!this->createSemaphoresAndFences())
        throw std::runtime_error("Cannot create semaphores and fences");


    m_isInit = true;
}

bool VInstance::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : const_validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return (false);
    }

    return (true);
}

std::vector<const char*> VInstance::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (const_enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    return extensions;
}

bool VInstance::createVulkanInstance()
{
    if (const_enableValidationLayers && !this->checkValidationLayerSupport())
        throw std::runtime_error("Validation layers requested not available");

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = m_name.c_str();
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "VALAG";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;


    std::vector<const char*> extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (const_enableValidationLayers)
    {
        createInfo.enabledLayerCount    = static_cast<uint32_t>(const_validationLayers.size());
        createInfo.ppEnabledLayerNames  = const_validationLayers.data();
    } else {
        createInfo.enabledLayerCount    = 0;
    }

    return (vkCreateInstance(&createInfo, nullptr, &m_vulkanInstance) == VK_SUCCESS);
}

bool VInstance::setupDebugCallback()
{
    if (!const_enableValidationLayers)
        return (true);

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = VInstance::debugCallback;

    return (CreateDebugReportCallbackEXT(m_vulkanInstance, &createInfo, nullptr, &m_debugCallback) == VK_SUCCESS);
}

bool VInstance::createSurface()
{
    return (glfwCreateWindowSurface(m_vulkanInstance, m_parentWindow, nullptr, &m_surface) == VK_SUCCESS);
}
bool VInstance::checkDeviceExtensionSupport(const VkPhysicalDevice &device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(const_deviceExtensions.begin(), const_deviceExtensions.end());

    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return (requiredExtensions.empty());
}

QueueFamilyIndices VInstance::findQueueFamilies(const VkPhysicalDevice &device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    bool foundedPreferedPresentQueue = false;

    for(uint32_t i = 0 ; i < queueFamilyCount ; ++i)
        if(queueFamilies[i].queueCount > 0)
        {
            VkBool32 presentSupport = false;

            if(!foundedPreferedPresentQueue)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

                if (presentSupport)
                    indices.presentFamily = i;
            }

            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
                if(presentSupport)
                    foundedPreferedPresentQueue = true;
            }

            if (foundedPreferedPresentQueue && indices.isComplete())
                break;
        }

    return indices;
}

SwapchainSupportDetails VInstance::querySwapchainSupport(const VkPhysicalDevice &device)
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

int VInstance::isDeviceSuitable(const VkPhysicalDevice &device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score = 1;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;

    score += deviceProperties.limits.maxImageDimension2D;

    /*if (!deviceFeatures.geometryShader)
        return 0;*/

    /** could change to just desactivate anisotropic filtering... but probably not useful since a gpu without it can certainly not run the engine**/
    if(!deviceFeatures.samplerAnisotropy)
        return 0;

    if(!checkDeviceExtensionSupport(device))
        return 0;

    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
    if(swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
        return 0;

    QueueFamilyIndices indices = findQueueFamilies(device);
    if(!indices.isComplete())
        return 0;

    m_queueFamilyIndices = indices;

    std::cout<<deviceProperties.deviceName<<" : "<<score<<std::endl;

    return score;
}

bool VInstance::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, nullptr);

    if(deviceCount == 0)
        return (false);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, devices.data());

    /**Probably need to allow user to pick device**/

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices)
    {
        int score = isDeviceSuitable(device);
        candidates.insert(std::make_pair(score, device));
    }

    if(candidates.rbegin()->first == 0)
        return (false);

    m_physicalDevice = candidates.rbegin()->second;

    return (true);
}

bool VInstance::createLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {m_queueFamilyIndices.graphicsFamily,
                                         m_queueFamilyIndices.presentFamily};

    float queuePriority = 1.0f;
    for (auto queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(const_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = const_deviceExtensions.data();

    if (const_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(const_validationLayers.size());
        createInfo.ppEnabledLayerNames = const_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        return (false);

    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily, 0, &m_presentQueue);

    return (true);
}

VkSurfaceFormatKHR VInstance::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

     for (const auto& availableFormat : availableFormats)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
        && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;

    return availableFormats[0];
}

VkPresentModeKHR VInstance::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
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

VkExtent2D VInstance::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int w, h;
    glfwGetWindowSize(m_parentWindow, &w, &h);
    VkExtent2D actualExtent = { static_cast<uint32_t>(w),
                                static_cast<uint32_t>(h)};

   /* actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));*/

    actualExtent.width  = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

bool VInstance::createSwapchain()
{
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(m_physicalDevice);

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

    uint32_t queueFamilyIndices[] = {(uint32_t) m_queueFamilyIndices.graphicsFamily,
                                     (uint32_t) m_queueFamilyIndices.presentFamily};

    if (m_queueFamilyIndices.graphicsFamily != m_queueFamilyIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
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

    if(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        return (false);

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    m_swapchainImageFormat = surfaceFormat.format;

    return (true);
}

bool VInstance::createImageViews()
{
    m_swapchainImageViews.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
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

        if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS)
            return (false);
    }

    return (true);
}

bool VInstance::createCommandPools()
{
    m_commandPools.resize(COMMANDPOOL_NBR_NAMES);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    for(size_t i = 0 ; i < COMMANDPOOL_NBR_NAMES ; ++i)
    {
        poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily; // Optional

        switch(i)
        {
            case COMMANDPOOL_DEFAULT:
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                break;

            case COMMANDPOOL_SHORTLIVED:
                poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                break;

            case COMMANDPOOL_TEXTURESLOADING:
                poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                break;

            default:
                poolInfo.flags = 0;
        }

        if(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPools[i]) != VK_SUCCESS)
            return (false);

    }

    return (true);
}

bool VInstance::createSemaphoresAndFences()
{
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
        if(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore[i]) != VK_SUCCESS)
            return (false);

        if(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            return (false);
    }


    if(vkCreateFence(m_device, &fenceInfo, nullptr, &m_graphicsQueueAccessFence) != VK_SUCCESS)
        return (false);


    return (true);
}

void VInstance::cleanup()
{
    vkDestroyFence(m_device, m_graphicsQueueAccessFence, nullptr);

    for (auto fence : m_inFlightFences)
        vkDestroyFence(m_device, fence, nullptr);

    for (auto semaphore : m_imageAvailableSemaphore)
        vkDestroySemaphore(m_device, semaphore, nullptr);

    for (auto commandPool : m_commandPools)
        vkDestroyCommandPool(m_device, commandPool, nullptr);

    /*vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyCommandPool(m_device, m_texturesLoadingCommandPool, nullptr);*/

    for (auto imageView : m_swapchainImageViews)
        vkDestroyImageView(m_device, imageView, nullptr);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    vkDestroyDevice(m_device, nullptr);

    if (const_enableValidationLayers)
     DestroyDebugReportCallbackEXT(m_vulkanInstance, m_debugCallback, nullptr);

    vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);
    vkDestroyInstance(m_vulkanInstance, nullptr);
}


VkDevice VInstance::getDevice()
{
    return m_device;
}

VkPhysicalDevice VInstance::getPhysicalDevice()
{
    return m_physicalDevice;
}


VkCommandPool VInstance::getCommandPool(CommandPoolName commandPoolName)
{
    /*if(commandPoolName == MAIN_COMMANDPOOL)
        return m_commandPool;
    else if(commandPoolName == TEXTURESLOADING_COMMANDPOOL)
        return m_texturesLoadingCommandPool;*/

    if(commandPoolName >= 0 && commandPoolName < COMMANDPOOL_NBR_NAMES)
        return m_commandPools[commandPoolName];

    return m_commandPools[0];
}

/*int VInstance::getGraphicsFamily()
{
    return m_queueFamilyIndices.graphicsFamily;
}*/

const std::vector<VkImageView> &VInstance::getSwapchainImageViews()
{
    return m_swapchainImageViews;
}

VkCommandBuffer VInstance::beginSingleTimeCommands(CommandPoolName commandPoolName)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    allocInfo.commandPool = this->getCommandPool(commandPoolName);

    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}


void VInstance::endSingleTimeCommands(VkCommandBuffer commandBuffer, CommandPoolName commandPoolName)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;


    vkWaitForFences(m_device, 1, &m_graphicsQueueAccessFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(m_device, 1, &m_graphicsQueueAccessFence);

    m_graphicsQueueAccessMutex.lock();
        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_graphicsQueueAccessFence);
    m_graphicsQueueAccessMutex.unlock();
    //vkQueueWaitIdle(m_graphicsQueue);

    vkWaitForFences(m_device, 1, &m_graphicsQueueAccessFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkFreeCommandBuffers(m_device, this->getCommandPool(commandPoolName), 1, &commandBuffer);
}



VInstance *VInstance::getCurrentInstance()
{
    return VInstance::static_currentInstance;
}

void VInstance::setCurrentInstance(VInstance *instance)
{
    VInstance::static_currentInstance = instance;
}


VKAPI_ATTR VkBool32 VKAPI_CALL VInstance::debugCallback(    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                            uint64_t obj, size_t location, int32_t code,
                                                            const char* layerPrefix, const char* msg, void* userData)
{
    Logger::error(msg);

    return VK_FALSE;
}

}
