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

    bool isComplete() {
        return graphicsFamily >= 0;
    }
};

class VInstance
{
    public:
        VInstance(const std::string name = "");
        virtual ~VInstance();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                            uint64_t obj, size_t location, int32_t code,
                                                            const char* layerPrefix, const char* msg, void* userData);

    protected:
        bool    checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        bool    createVulkanInstance();
        bool    setupDebugCallback();
        QueueFamilyIndices  findQueueFamilies(const VkPhysicalDevice &device);
        int     isDeviceSuitable(const VkPhysicalDevice &device);
        bool    pickPhysicalDevice();
        bool    createLogicalDevice();

        void init();
        void cleanup();

    private:
        std::string m_name;

        VkInstance          m_vulkanInstance;
        VkDebugReportCallbackEXT m_debugCallback;
        VkPhysicalDevice    m_physicalDevice;
        QueueFamilyIndices  m_queueFamilyIndices;
        VkDevice            m_device;
        VkQueue             m_graphicsQueue;
};

}

#endif // VINSTANCE_H
