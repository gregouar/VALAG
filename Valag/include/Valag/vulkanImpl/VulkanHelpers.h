#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <Vulkan/Vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <array>

#include "Valag/Types.h"

namespace vlg
{

struct Vertex2D
{
    glm::vec2 pos;
    //glm::vec4 color;
    glm::vec2 tex;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

struct ViewUBO {
    glm::mat4 view;
};
struct ModelUBO {
    glm::mat4 model;
    glm::vec4 color;
    glm::vec2 texPos;
    glm::vec2 texExt;
};

class VulkanHelpers
{

public:
    static std::vector<char> readFile(const std::string& filename);

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    static bool copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);

    /** I'll need to move this to VBuffersAllocator **/
    static bool createImage(uint32_t width, uint32_t height, uint32_t layerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    static void transitionImageLayout(VkImage image, uint32_t layer, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                      CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);
    static void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer,
                                   CommandPoolName commandPoolName = COMMANDPOOL_SHORTLIVED);

    static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount);

    static VkShaderModule createShaderModule(const std::vector<char>& code);

};

}

#endif // VULKANHELPERS_H

