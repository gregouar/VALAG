#include "Valag/gfx/Sprite.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Valag/core/VApp.h"
#include "Valag/gfx/VInstance.h"
#include "Valag/gfx/DefaultRenderer.h"

namespace vlg
{

Sprite::Sprite() :
    m_size({0,0}),
    m_position({0,0}),
    m_texture(-1),
    m_texturePosition({0,0}),
    m_textureExtent({0,0}),
    m_needToCreateBuffers(true),
    m_needToUpdateDrawCommandBuffer(true),
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
            //m_needToUpdateUBO = true;
            m_needToUpdateModel = true;
        m_size = size;
    }

}

void Sprite::setPosition(glm::vec2 position)
{
    if(m_position != position)
            //m_needToUpdateUBO = true;
            m_needToUpdateModel = true;
    m_position = position;
}

void Sprite::setTexture()
{

}

void Sprite::setTextureRect(glm::vec2 position, glm::vec2 extent)
{
    if(m_texturePosition != position || extent != m_textureExtent)
            //m_needToUpdateUBO = true;
            m_needToUpdateModel = true;

    m_texturePosition = position;
    m_textureExtent = extent;
}

void Sprite::createAllBuffers()
{
    this->createVertexBuffer();
    //this->createModelUBO();
    this->createDrawCommandBuffer();

    m_needToCreateBuffers = false;
}

void Sprite::createDrawCommandBuffer()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No vulkan instance in createDrawCommandBuffer()");

    m_drawCommandBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkanInstance->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = (uint32_t) m_drawCommandBuffers.size();


    if (vkAllocateCommandBuffers(vulkanInstance->getDevice(), &allocInfo, m_drawCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");


   // if (vkAllocateCommandBuffers(vulkanInstance->getDevice(), &allocInfo, &m_drawCommandBuffer) != VK_SUCCESS)
     //   throw std::runtime_error("Failed to allocate command buffers");
}

void Sprite::createVertexBuffer()
{
    VInstance *vulkanInstance = VInstance::getCurrentInstance();

    if(vulkanInstance == nullptr)
        throw std::runtime_error("No vulkan instance in createVertexBuffer()");

    VkDevice device = vulkanInstance->getDevice();

    std::vector<Vertex2D> vertices =
            {
                {glm::vec2(0,1), {1.0f, 0.0f, 0.0f}, m_texturePosition+glm::vec2(0,m_textureExtent.y)},
                {glm::vec2(0,0), {0.0f, 1.0f, 0.0f}, m_texturePosition},
                {glm::vec2(1,1), {0.0f, 0.0f, 1.0f}, m_texturePosition+m_textureExtent},
                {glm::vec2(1,0), {1.0f, 1.0f, 1.0f}, m_texturePosition+glm::vec2(m_textureExtent.x,0)}
            };

    VkDeviceSize bufferSize = sizeof(Vertex2D) * vertices.size();

    VkBuffer stagingBuffer;
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
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    m_creatingVInstance = vulkanInstance;
}

/*void Sprite::createModelUBO()
{
    VkDeviceSize bufferSize = sizeof(ModelUBO);

    m_modelBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_modelBuffersMemory.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < m_modelBuffers.size(); ++i)
    {
        VulkanHelpers::createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    m_modelBuffers[i], m_modelBuffersMemory[i]);

    }

    m_needToUpdateUBO = true;
    m_currentFrame = 0;
}*/

/*void Sprite::updateModelUBO()
{
    m_currentFrame = (m_currentFrame + 1) % VApp::MAX_FRAMES_IN_FLIGHT;

    if(m_creatingVInstance == nullptr)
        throw std::runtime_error("No vulkan instance in updateModelUBO()");

    VkDevice device = m_creatingVInstance->getDevice();

    ModelUBO modelUBO = {};
    modelUBO.model = glm::mat4(m_size.x,0,0,m_position.x,
                             0,m_size.y, 0, m_position.y,
                             0,0,1,0,
                             0,0,0,1 );

    void* data;
    vkMapMemory(device, m_modelBuffersMemory[m_currentFrame], 0, sizeof(modelUBO), 0, &data);
        memcpy(data, &modelUBO, sizeof(modelUBO));
    vkUnmapMemory(device, m_modelBuffersMemory[m_currentFrame]);

    m_needToUpdateUBO = false;
}*/

VkCommandBuffer Sprite::getDrawCommandBuffer(DefaultRenderer *renderer/*VkPipeline pipeline*/, size_t currentFrame, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    //m_currentFrame = (m_currentFrame + 1) % VApp::MAX_FRAMES_IN_FLIGHT;

    if(m_needToCreateBuffers)
    {
        this->createAllBuffers();
        m_modelUBOIndex = renderer->allocModelUBO();
        m_needToUpdateModel = true;
    }

    ///Should move this to method
    if(m_needToUpdateModel)
    {
        ModelUBO modelUBO = {};
        modelUBO.model = glm::mat4(1.0);
        modelUBO.model = glm::translate(modelUBO.model, glm::vec3(m_position.x, m_position.y, 0.0));
        modelUBO.model = glm::scale(modelUBO.model, glm::vec3(m_size.x, m_size.y, 1.0));

        renderer->updateModelUBO(m_modelUBOIndex, &modelUBO);
    }

    /*if(m_needToUpdateUBO)
        this->updateModelUBO();*/
    /*if(m_needToCreateVertexBuffer)
        this->createVertexBuffer();

    if(m_needToCreateDrawCommandBuffer)
        this->createDrawCommandBuffer();*/

    if(m_needToUpdateDrawCommandBuffer)
        this->recordDrawCommandBuffers(renderer/*pipeline*/, renderPass, subpass, framebuffer);

    return m_drawCommandBuffers[currentFrame];

}

void Sprite::recordDrawCommandBuffers(DefaultRenderer *renderer,/*VkPipeline pipeline,*/ VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
    {
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

        if (vkBeginCommandBuffer(m_drawCommandBuffers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin recording command buffer");

        renderer->bindAllUBOs(m_drawCommandBuffers[i],i,m_modelUBOIndex);

        VkBuffer vertexBuffers[] = {m_vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(m_drawCommandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdDraw(m_drawCommandBuffers[i], 4, 1, 0, 0);

        if (vkEndCommandBuffer(m_drawCommandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");
    }

    m_needToUpdateDrawCommandBuffer = false;
}

void Sprite::cleanup()
{
    if(m_creatingVInstance == nullptr)
        return;

    VkDevice device = m_creatingVInstance->getDevice();

        /**Need to add memoryAllocator associated to instance that clean itself at the end**/
    if(m_vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, m_vertexBuffer, nullptr);
        vkFreeMemory(device, m_vertexBufferMemory, nullptr);
    }

    /*for(auto ubo : m_modelBuffers)
        vkDestroyBuffer(device, ubo, nullptr);

    for(auto uboMemory : m_modelBuffersMemory)
        vkFreeMemory(device, uboMemory, nullptr);*/
}

}
