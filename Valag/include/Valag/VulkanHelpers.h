#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Valag/Types.h"

namespace vlg
{

class VulkanHelpers
{

public:
    static std::vector<char> readFile(const std::string& filename);

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    static bool createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    static void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                      CommandPoolName commandPoolName = MAIN_COMMANDPOOL);
    static void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, CommandPoolName commandPoolName = MAIN_COMMANDPOOL);

    static VkImageView createImageView(VkImage image, VkFormat format);

    static VkShaderModule createShaderModule(const std::vector<char>& code);

};

}

#endif // VULKANHELPERS_H

