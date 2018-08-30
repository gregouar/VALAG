#ifndef VINSTANCE_H
#define VINSTANCE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <mutex>

#include "Valag/vulkanImpl/VulkanHelpers.h"
#include "Valag/utils/Singleton.h"

namespace vlg
{

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool isComplete() {
        return graphicsFamily >= 0
            && presentFamily >= 0;
    }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


class VInstance : public Singleton<VInstance>
{
    friend class VApp;
    friend class RenderWindow;
    friend class VBuffersAllocator;
    friend class VulkanHelpers;

    friend class Singleton<VInstance>;

    public:
        static VkDevice device();
        static VkPhysicalDevice physicalDevice();
        static VkCommandPool commandPool(CommandPoolName commandPoolName = COMMANDPOOL_DEFAULT);

        static void submitToGraphicsQueue(VkSubmitInfo &infos, VkFence fence = VK_NULL_HANDLE);
        static void presentQueue(VkPresentInfoKHR &infos);
        static void waitDeviceIdle();

        bool isInitialized();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                            uint64_t obj, size_t location, int32_t code,
                                                            const char* layerPrefix, const char* msg, void* userData);

    protected:
        VInstance();
        virtual ~VInstance();


        bool    checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        bool    createVulkanInstance();
        bool    setupDebugCallback();

        bool                    checkDeviceExtensionSupport(const VkPhysicalDevice &device);
        QueueFamilyIndices      findQueueFamilies(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
        SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
        int     isDeviceSuitable(const VkPhysicalDevice &device,VkSurfaceKHR &surface);
        bool    pickPhysicalDevice(VkSurfaceKHR &surface);

        bool    createLogicalDevice();
        bool    createCommandPools();
        bool    createSemaphoresAndFences();

        void init(VkSurfaceKHR &surface);
        void cleanup();

        VkDevice            getDevice();
        VkPhysicalDevice    getPhysicalDevice();
        VkCommandPool       getCommandPool(CommandPoolName commandPoolName = COMMANDPOOL_DEFAULT);

        QueueFamilyIndices  getQueueFamilyIndices();
        VkInstance          getVulkanInstance();

        VkCommandBuffer beginSingleTimeCommands(CommandPoolName commandPoolName = COMMANDPOOL_DEFAULT);
        void            endSingleTimeCommands(VkCommandBuffer commandBuffer, CommandPoolName commandPoolName = COMMANDPOOL_DEFAULT);

    private:
        std::string m_name;

        VkInstance          m_vulkanInstance;
        VkDebugReportCallbackEXT m_debugCallback;
        VkPhysicalDevice    m_physicalDevice;
        VkDevice            m_device;

        QueueFamilyIndices  m_queueFamilyIndices;
        VkQueue             m_graphicsQueue;
        VkQueue             m_presentQueue;

        VkFence             m_graphicsQueueAccessFence;
        std::mutex          m_graphicsQueueAccessMutex;

        ///I should try to do one command pool per frame (and reset) ?
        std::vector<VkCommandPool>  m_commandPools;

        bool m_isInit;

    public:

        static const std::vector<const char*> const_validationLayers;
        static const std::vector<const char*> const_deviceExtensions;

};

}

#endif // VINSTANCE_H
