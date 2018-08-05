#include "Valag/gfx/VInstance.h"

#include <cstring>
#include <map>

#include "Valag/VulkanExtProxies.h"

#include "Valag/utils/Logger.h"

namespace vlg
{


const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


VInstance::VInstance(const std::string name) :
    m_name(name),
    m_physicalDevice(VK_NULL_HANDLE)
{
    this->init();
}

VInstance::~VInstance()
{
    this->cleanup();
}


void VInstance::init()
{
    if(!this->createVulkanInstance())
        throw std::runtime_error("Can not create Vulkan instance");

    if(!this->setupDebugCallback())
        throw std::runtime_error("Can not setup debug callback");

    if(!this->pickPhysicalDevice())
        throw std::runtime_error("Can not find suitable physical device");

    if(!this->createLogicalDevice())
        throw std::runtime_error("Can not create logical device");
}

bool VInstance::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
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

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
}

bool VInstance::createVulkanInstance()
{
    if (enableValidationLayers && !this->checkValidationLayerSupport())
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

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount    = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames  = validationLayers.data();
    } else {
        createInfo.enabledLayerCount    = 0;
    }

    return (vkCreateInstance(&createInfo, nullptr, &m_vulkanInstance) == VK_SUCCESS);
}

bool VInstance::setupDebugCallback()
{
    if (!enableValidationLayers)
        return (true);

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = VInstance::debugCallback;

    return (CreateDebugReportCallbackEXT(m_vulkanInstance, &createInfo, nullptr, &m_debugCallback) == VK_SUCCESS);
}

QueueFamilyIndices VInstance::findQueueFamilies(const VkPhysicalDevice &device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for(uint32_t i = 0 ; i < queueFamilyCount ; ++i)
    {
        if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        if (indices.isComplete())
            break;
    }

    return indices;
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

    if (!deviceFeatures.geometryShader)
        return 0;

    QueueFamilyIndices indices = findQueueFamilies(device);
    if(!indices.isComplete())
        return 0;

    m_queueFamilyIndices = indices;

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
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = 0;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        return (false);

    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily, 0, &m_graphicsQueue);

    return (true);
}

void VInstance::cleanup()
{
    vkDestroyDevice(m_device, nullptr);

    if (enableValidationLayers)
        DestroyDebugReportCallbackEXT(m_vulkanInstance, m_debugCallback, nullptr);

    vkDestroyInstance(m_vulkanInstance, nullptr);
}


VKAPI_ATTR VkBool32 VKAPI_CALL VInstance::debugCallback(    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                            uint64_t obj, size_t location, int32_t code,
                                                            const char* layerPrefix, const char* msg, void* userData)
{
    std::cout<<"HAHAHAHD"<<std::endl;
    Logger::error(msg);

    return VK_FALSE;
}

}
