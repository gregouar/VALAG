#include "Valag/gfx/Sprite.h"

#include "Valag/gfx/VInstance.h"

namespace vlg
{

Sprite::Sprite() :
    m_size({0,0}),
    m_position({0,0}),
    m_texture(-1),
    m_texturePosition({0,0}),
    m_textureExtent({0,0}),
    m_needToCreateDrawCommandBuffer(true),
    m_needToUpdateDrawCommandBuffer(true),
    m_needToCreateVertexBuffer(true),
    m_creatingVInstance(nullptr),
    m_vertexBuffer(VK_NULL_HANDLE)
{
    //ctor
}

Sprite::~Sprite()
{
    this->cleanup();
}


void Sprite::setSize(glm::vec2 size)
{
    if(size.x >= 0 && size.y >= 0)
    {
        if(m_size != size)
            m_needToCreateVertexBuffer = true;
        m_size = size;
    }

}

void Sprite::setPosition(glm::vec2 position)
{
    if(m_position != position)
        m_needToCreateVertexBuffer = true;
    m_position = position;
}

void Sprite::setTexture()
{

}

void Sprite::setTextureRect(glm::vec2 position, glm::vec2 extent)
{
    if(m_texturePosition != position || extent != m_textureExtent)
        m_needToCreateVertexBuffer = true;

    m_texturePosition = position;
    m_textureExtent = extent;
}


void Sprite::createDrawCommandBuffer()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No vulkan instance in createDrawCommandBuffer()");

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkanInstance->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(vulkanInstance->getDevice(), &allocInfo, &m_drawCommandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");

    m_needToCreateDrawCommandBuffer = false;
}

void Sprite::createVertexBuffer()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No vulkan instance in createVertexBuffer()");

    VkDevice device = vulkanInstance->getDevice();

    std::vector<Vertex2D> vertices =
            {
                {m_position+glm::vec2(0,m_size.y), {1.0f, 0.0f, 0.0f}, m_texturePosition+glm::vec2(0,m_textureExtent.y)},
                {m_position, {0.0f, 1.0f, 0.0f}, m_texturePosition},
                {m_position+m_size, {0.0f, 0.0f, 1.0f}, m_texturePosition+m_textureExtent},
                {m_position+glm::vec2(m_size.x,0), {1.0f, 1.0f, 1.0f}, m_texturePosition+glm::vec2(m_textureExtent.x,0)}
            };

    VkDeviceSize bufferSize = sizeof(Vertex2D) * vertices.size();


    VulkanHelpers::createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                m_vertexBuffer, m_vertexBufferMemory);

    void* data;
    vkMapMemory(device, m_vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, m_vertexBufferMemory);



    /**VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VulkanHelpers::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    VulkanHelpers::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                m_vertexBuffer, m_vertexBufferMemory);

    VulkanHelpers::copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);**/

    m_needToCreateVertexBuffer = false;
    m_creatingVInstance = vulkanInstance;
}

VkCommandBuffer Sprite::recordDrawCommandBuffer(VkPipeline pipeline, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    if(m_needToCreateVertexBuffer)
        this->createVertexBuffer();

    if(m_needToCreateDrawCommandBuffer)
        this->createDrawCommandBuffer();

    if(!m_needToUpdateDrawCommandBuffer)
        return m_drawCommandBuffer;

    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = renderPass;
    inheritanceInfo.subpass = subpass;
    inheritanceInfo.framebuffer = framebuffer;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags = 0;
    inheritanceInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional

    if (vkBeginCommandBuffer(m_drawCommandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer");

    vkCmdBindPipeline(m_drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(m_drawCommandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(m_drawCommandBuffer, 4, 1, 0, 0);

    if (vkEndCommandBuffer(m_drawCommandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");

    m_needToUpdateDrawCommandBuffer = false;

    return m_drawCommandBuffer;
}

void Sprite::cleanup()
{
    if(m_creatingVInstance == nullptr)
        return;

        /**Need to add memoryAllocator associated to instance that clean itself at the end**/
    if(m_vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_creatingVInstance->getDevice(), m_vertexBuffer, nullptr);
        vkFreeMemory(m_creatingVInstance->getDevice(), m_vertexBufferMemory, nullptr);
    }
}

}
