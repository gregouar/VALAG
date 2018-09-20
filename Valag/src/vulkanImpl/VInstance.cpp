#include "Valag/vulkanImpl/VInstance.h"

#include <cstring>
#include <map>
#include <set>
#include <glm/glm.hpp>

#include "Valag/vulkanImpl/VulkanExtProxies.h"

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


VInstance::VInstance() :
    m_physicalDevice(VK_NULL_HANDLE),
    m_device(VK_NULL_HANDLE),
    m_graphicsQueueAccessFence(VK_NULL_HANDLE),
    m_isInit(false)
{
    if(!this->createVulkanInstance())
        throw std::runtime_error("Cannot create Vulkan instance");

    if(!this->setupDebugCallback())
        throw std::runtime_error("Cannot setup debug callback");
}

VInstance::~VInstance()
{
    this->cleanup();

    if (const_enableValidationLayers)
        DestroyDebugReportCallbackEXT(m_vulkanInstance, m_debugCallback, nullptr);
    vkDestroyInstance(m_vulkanInstance, nullptr);
}


VkDevice VInstance::device()
{
    if(!VInstance::instance()->isInitialized())
        return VK_NULL_HANDLE;
    return VInstance::instance()->getDevice();
}

VkPhysicalDevice VInstance::physicalDevice()
{
    if(!VInstance::instance()->isInitialized())
        return VK_NULL_HANDLE;
    return VInstance::instance()->getPhysicalDevice();
}

VkCommandPool VInstance::commandPool(CommandPoolName commandPoolName)
{
    if(!VInstance::instance()->isInitialized())
        return VK_NULL_HANDLE;
    return VInstance::instance()->getCommandPool(commandPoolName);
}

void VInstance::submitToGraphicsQueue(VkSubmitInfo &infos, VkFence fence)
{
    VInstance::instance()->m_graphicsQueueAccessMutex.lock();
        if (vkQueueSubmit(VInstance::instance()->m_graphicsQueue, 1, &infos, fence) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer");
    VInstance::instance()->m_graphicsQueueAccessMutex.unlock();
}

void VInstance::submitToGraphicsQueue(std::vector<VkSubmitInfo> &infos, VkFence fence)
{
    VInstance::instance()->m_graphicsQueueAccessMutex.lock();
        if (vkQueueSubmit(VInstance::instance()->m_graphicsQueue, infos.size(), infos.data(), fence) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer");
    VInstance::instance()->m_graphicsQueueAccessMutex.unlock();
}

void VInstance::presentQueue(VkPresentInfoKHR &infos)
{
    vkQueuePresentKHR(VInstance::instance()->m_presentQueue, &infos);
}

void VInstance::waitDeviceIdle()
{
    vkDeviceWaitIdle(VInstance::device());
}

bool VInstance::isInitialized()
{
    return m_isInit;
}

void VInstance::init(VkSurfaceKHR &surface)
{
    if(!this->pickPhysicalDevice(surface))
        throw std::runtime_error("Cannot find suitable physical device");

    if(!this->createLogicalDevice())
        throw std::runtime_error("Cannot create logical device");

    if(!this->createCommandPools())
        throw std::runtime_error("Cannot create command pools");

    if(!this->createSingleTimeCmbs())
        throw std::runtime_error("Cannot create single time command buffers");

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
    appInfo.apiVersion          = VK_API_VERSION_1_1;

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

QueueFamilyIndices VInstance::findQueueFamilies(const VkPhysicalDevice &device, VkSurfaceKHR &surface)
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
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

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

SwapchainSupportDetails VInstance::querySwapchainSupport(const VkPhysicalDevice &device, VkSurfaceKHR &surface)
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

int VInstance::isDeviceSuitable(const VkPhysicalDevice &device,VkSurfaceKHR &surface)
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

    /** This could be a serious problem... 200 on intel chipset is really low**/
    //std::cout<<deviceProperties.limits.maxPerStageDescriptorSampledImages<<std::endl;
    //std::cout<<deviceProperties.limits.maxImageArrayLayers<<std::endl;
    if(deviceProperties.limits.maxPerStageDescriptorSampledImages < VTexturesManager::MAX_TEXTURES_ARRAY_SIZE)
        return 0;

    if(!deviceFeatures.shaderSampledImageArrayDynamicIndexing)
        return 0;

    if(!checkDeviceExtensionSupport(device))
        return 0;

    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device,surface);
    if(swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
        return 0;

    QueueFamilyIndices indices = findQueueFamilies(device,surface);
    if(!indices.isComplete())
        return 0;

    m_queueFamilyIndices = indices;

    std::cout<<deviceProperties.deviceName<<" : "<<score<<std::endl;

    return score;
}

bool VInstance::pickPhysicalDevice(VkSurfaceKHR &surface)
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
        int score = isDeviceSuitable(device,surface);
        candidates.insert(std::make_pair(score, device));
    }

    if(candidates.rbegin()->first == 0)
        return (false);

    m_physicalDevice = candidates.rbegin()->second;
   // m_physicalDevice = candidates.begin()->second;

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
    deviceFeatures.independentBlend  = VK_TRUE; ///Do I really need it ?

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(const_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = const_deviceExtensions.data();

    if (const_enableValidationLayers)
    {
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

bool VInstance::createCommandPools()
{
    m_commandPools.resize(COMMANDPOOL_NBR_NAMES);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    for(size_t i = 0 ; i < COMMANDPOOL_NBR_NAMES ; ++i)
    {
        poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily; // Optional

        ///I could add a command pool for never resetted cmbs
        switch(i)
        {
            case COMMANDPOOL_DEFAULT:
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                break;

            case COMMANDPOOL_SHORTLIVED:
                poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT ;
                break;

            case COMMANDPOOL_TEXTURESLOADING:
                poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT ;
                break;

            default:
                poolInfo.flags = 0;
        }

        if(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPools[i]) != VK_SUCCESS)
            return (false);

    }

    return (true);
}

bool VInstance::createSingleTimeCmbs()
{
    m_singleTimeCmb.resize(COMMANDPOOL_NBR_NAMES);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    for(size_t i = 0 ; i < m_commandPools.size() ; ++i)
    {
        allocInfo.commandPool = m_commandPools[i];
        if(vkAllocateCommandBuffers(m_device, &allocInfo, &m_singleTimeCmb[i]) != VK_SUCCESS)
            return (false);
    }

    return (true);
}

bool VInstance::createSemaphoresAndFences()
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if(vkCreateFence(m_device, &fenceInfo, nullptr, &m_graphicsQueueAccessFence) != VK_SUCCESS)
        return (false);


    return (true);
}

void VInstance::cleanup()
{
    if(m_graphicsQueueAccessFence != VK_NULL_HANDLE)
        vkDestroyFence(m_device, m_graphicsQueueAccessFence, nullptr);
    m_graphicsQueueAccessFence = VK_NULL_HANDLE;

    for (auto commandPool : m_commandPools)
        vkDestroyCommandPool(m_device, commandPool, nullptr);
    m_commandPools.clear();

    if(m_device != VK_NULL_HANDLE)
        vkDestroyDevice(m_device, nullptr);
    m_device = VK_NULL_HANDLE;

    m_isInit = false;
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
    if(commandPoolName >= 0 && commandPoolName < COMMANDPOOL_NBR_NAMES)
        return m_commandPools[commandPoolName];

    return m_commandPools[0];
}

QueueFamilyIndices VInstance::getQueueFamilyIndices()
{
    return m_queueFamilyIndices;
}

VkInstance VInstance::getVulkanInstance()
{
    return m_vulkanInstance;
}

VkCommandBuffer VInstance::beginSingleTimeCommands(CommandPoolName commandPoolName)
{
    if(commandPoolName == COMMANDPOOL_DEFAULT)
    {
        Logger::error("Cannot use single time commands in default commandpool");
        return VK_NULL_HANDLE;
    }


    /*VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    allocInfo.commandPool = this->getCommandPool(commandPoolName);

    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);*/

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_singleTimeCmb[commandPoolName], &beginInfo);

    return m_singleTimeCmb[commandPoolName];
}


void VInstance::endSingleTimeCommands(VkCommandBuffer commandBuffer/*, CommandPoolName commandPoolName*/)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    m_graphicsQueueAccessMutex.lock();

    vkWaitForFences(m_device, 1, &m_graphicsQueueAccessFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(m_device, 1, &m_graphicsQueueAccessFence);

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_graphicsQueueAccessFence);


    vkWaitForFences(m_device, 1, &m_graphicsQueueAccessFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
   // vkFreeCommandBuffers(m_device, this->getCommandPool(commandPoolName), 1, &commandBuffer);
    m_graphicsQueueAccessMutex.unlock();

    //vkQueueWaitIdle(m_graphicsQueue);

}


VKAPI_ATTR VkBool32 VKAPI_CALL VInstance::debugCallback(    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                            uint64_t obj, size_t location, int32_t code,
                                                            const char* layerPrefix, const char* msg, void* userData)
{
    Logger::error(msg);

    return VK_FALSE;
}

}
