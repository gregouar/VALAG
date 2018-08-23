#ifndef VINSTANCE_H
#define VINSTANCE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <mutex>

#include "Valag/vulkanImpl/VulkanHelpers.h"

namespace vlg
{

class VApp;

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


class VInstance
{
    friend class TextureAsset;
    friend class VulkanHelpers;
    friend class DefaultRenderer;
    friend class SceneRenderer;
    friend class Sprite;
    friend class DynamicUBO;
    friend class VMemoryAllocator;

    public:
        VInstance(GLFWwindow *window, const std::string name = "");
        virtual ~VInstance();

        uint32_t acquireNextImage();
        void submitToGraphicsQueue(VkCommandBuffer commandBuffer, VkSemaphore finishedRenderingSemaphore);
        void presentQueue();
        void waitDeviceIdle();
        size_t getCurrentFrameIndex();

        bool isInitialized();

        void setActive();

        VkExtent2D getSwapchainExtent();
        VkFormat getSwapchainImageFormat();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                            uint64_t obj, size_t location, int32_t code,
                                                            const char* layerPrefix, const char* msg, void* userData);

    protected:
        bool    checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        bool    createVulkanInstance();
        bool    setupDebugCallback();
        bool    createSurface();
        bool    checkDeviceExtensionSupport(const VkPhysicalDevice &device);
        QueueFamilyIndices  findQueueFamilies(const VkPhysicalDevice &device);
        SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice &device);
        int     isDeviceSuitable(const VkPhysicalDevice &device);
        bool    pickPhysicalDevice();
        bool    createLogicalDevice();
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
        bool    createSwapchain();
        bool    createImageViews();
        bool    createCommandPools();
        bool    createSemaphoresAndFences();

        void init();
        void cleanup();

        VkDevice getDevice();
        VkPhysicalDevice getPhysicalDevice();
        VkCommandPool getCommandPool(CommandPoolName commandPoolName = COMMANDPOOL_DEFAULT);
        //int getGraphicsFamily();

        const std::vector<VkImageView> &getSwapchainImageViews();

        VkCommandBuffer beginSingleTimeCommands(CommandPoolName commandPoolName = COMMANDPOOL_DEFAULT);
        void endSingleTimeCommands(VkCommandBuffer commandBuffer, CommandPoolName commandPoolName = COMMANDPOOL_DEFAULT);

        static VInstance *getCurrentInstance();
        static void setCurrentInstance(VInstance *instance);

    private:
        std::string m_name;
        GLFWwindow *m_parentWindow;

        VkInstance          m_vulkanInstance;
        VkDebugReportCallbackEXT m_debugCallback;
        VkSurfaceKHR        m_surface;
        VkPhysicalDevice    m_physicalDevice;
        VkDevice            m_device;

        QueueFamilyIndices  m_queueFamilyIndices;
        VkQueue             m_graphicsQueue;
        VkQueue             m_presentQueue;

        VkFence             m_graphicsQueueAccessFence;
        std::mutex          m_graphicsQueueAccessMutex;

        VkSwapchainKHR      m_swapchain;
        std::vector<VkImage>     m_swapchainImages;
        std::vector<VkImageView> m_swapchainImageViews;
        VkFormat    m_swapchainImageFormat;
        VkExtent2D  m_swapchainExtent;

        /*VkCommandPool m_commandPool,
                      m_texturesLoadingCommandPool;*/

        ///I should try to do one command pool per frame (and reset) ?
        std::vector<VkCommandPool>  m_commandPools;

        uint32_t    m_curImageIndex;
        size_t      m_currentFrame;
        std::vector<VkSemaphore>                 m_imageAvailableSemaphore;
        std::vector<std::vector<VkSemaphore>>    m_finishedRenderingSemaphores;
        std::vector<VkFence>                     m_inFlightFences;

        static VInstance *static_currentInstance;

        bool m_isInit;

    public:

        static const std::vector<const char*> const_validationLayers;
        static const std::vector<const char*> const_deviceExtensions;

};

}

#endif // VINSTANCE_H
