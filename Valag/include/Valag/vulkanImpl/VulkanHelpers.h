#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <Vulkan/Vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <array>

#include "Valag/Types.h"
#include "Valag/vulkanImpl/VMemoryAllocator.h"

namespace vlg
{

struct VImage
{
    VMemory memory;
    VkImage vkImage;
};

struct VFramebufferAttachmentType
{
    VkFormat        format;
    VkImageLayout   layout;
};

struct VFramebufferAttachment
{
    VImage          image;
    VkImageView     view;
    VkExtent2D      extent;
    VFramebufferAttachmentType type;
};

class VulkanHelpers
{

public:
    static std::vector<char> readFile(const std::string& filename);

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    static bool copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);


    static bool createImage(uint32_t width, uint32_t height, uint32_t layerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VImage &image/*VkImage& image, VkDeviceMemory& imageMemory*/);
    static bool createImage(uint32_t width, uint32_t height, uint32_t layerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    static void destroyImage(VImage image);

    static void transitionImageLayout(VkImage image, uint32_t layer, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                      CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);

    static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount);

    static bool createAttachment(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VFramebufferAttachment &attachment);
    static void destroyAttachment(VFramebufferAttachment attachment);

    static VkShaderModule createShaderModule(const std::vector<char>& code);

    /// Need to add swizzle
    static void takeScreenshot(const VFramebufferAttachment &source, const std::string &filepath);

};

}

#endif // VULKANHELPERS_H

