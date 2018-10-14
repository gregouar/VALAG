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
    VImage() : vkImage(VK_NULL_HANDLE){}
    VMemory memory;
    VkImage vkImage;
    uint32_t layerCount;
    uint32_t mipsCount;
    VkFormat format;
};

struct VFramebufferAttachmentType
{
    VkFormat        format;
    VkImageLayout   layout;
};

struct VFramebufferAttachment
{
    VFramebufferAttachment() : view(VK_NULL_HANDLE){}

    VImage                      image;
    VkImageView                 view; //View with all mips (use only for uniform or with mipsCount = 1)
    std::vector<VkImageView>    mipViews;
    VkExtent2D                  extent;
    VFramebufferAttachmentType  type;
};

class VulkanHelpers
{

public:
    static std::vector<char> readFile(const std::string& filename);

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    static bool copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);


    static bool createImage(uint32_t width, uint32_t height, uint32_t layerCount,
                            VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VImage &image);
    static bool createImage(uint32_t width, uint32_t height, uint32_t layerCount, uint32_t mipsCount,
                            VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VImage &image);
    static bool createImage(uint32_t width, uint32_t height, uint32_t layerCount,
                            VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    static void destroyImage(VImage &image);

    static void transitionImageLayout(VImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                                      CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);
    static void transitionImageLayout(VImage image, uint32_t layer, VkImageLayout oldLayout, VkImageLayout newLayout,
                                      CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);
    static void transitionImageLayout(VkImage image, uint32_t layer, uint32_t mipsCount, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                      CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);

    static VkImageView createImageView(VImage image, VkImageAspectFlags aspectFlags);
    static VkImageView createImageView(VImage image, VkImageAspectFlags aspectFlags, uint32_t mipLevel);
    static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                       uint32_t layerCount = 1, uint32_t mipsCount = 1, uint32_t mipLevel = 0);

    static bool createAttachment(uint32_t width, uint32_t height, uint32_t mipsCount,
                                 VkFormat format, VkImageUsageFlags usage, VFramebufferAttachment &attachment);
    static bool createAttachment(uint32_t width, uint32_t height,
                                 VkFormat format, VkImageUsageFlags usage, VFramebufferAttachment &attachment);

    static void destroyAttachment(VFramebufferAttachment &attachment);

    static VkShaderModule createShaderModule(const std::vector<char>& code);

    /// Need to add swizzle
    static void takeScreenshot(const VFramebufferAttachment &source, const std::string &filepath);

};

}

#endif // VULKANHELPERS_H

