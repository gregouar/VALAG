#ifndef VINSTANCE_H
#define VINSTANCE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

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
    public:
        VInstance(GLFWwindow *window, const std::string name = "");
        virtual ~VInstance();

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

        void init();
        void cleanup();

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

        VkSwapchainKHR       m_swapchain;
        std::vector<VkImage>     m_swapchainImages;
        std::vector<VkImageView> m_swapchainImageViews;
        VkFormat    m_swapchainImageFormat;
        VkExtent2D  m_swapchainExtent;

    public:

        static const std::vector<const char*> validationLayers;
        static const std::vector<const char*> deviceExtensions;

};

}

#endif // VINSTANCE_H
