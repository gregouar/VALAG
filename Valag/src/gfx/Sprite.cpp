#include "Valag/gfx/Sprite.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Valag/core/VApp.h"
#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/gfx/DefaultRenderer.h"

namespace vlg
{

Sprite::Sprite() :
    m_size({0,0}),
    m_position({0,0}),
    m_texture(0),
    m_texturePosition({0,0}),
    m_textureExtent({1,1}),
    m_needToCreateBuffers(true)
{
    m_needToUpdateDrawCommandBuffer = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, true);
    m_needToAllocModel = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, true);
    m_needToUpdateModel = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, true);
    m_modelBufferVersion = std::vector<size_t> (VApp::MAX_FRAMES_IN_FLIGHT, 0);
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
            for(auto b : m_needToUpdateModel) b = true;
        m_size = size;
    }

}

void Sprite::setPosition(glm::vec2 position)
{
    if(m_position != position)
            //m_needToUpdateUBO = true;
            for(auto b : m_needToUpdateModel) b = true;
    m_position = position;
}

void Sprite::setTexture(AssetTypeID textureID)
{
    m_texture = textureID;
}

void Sprite::setTextureRect(glm::vec2 position, glm::vec2 extent)
{
    if(m_texturePosition != position || extent != m_textureExtent)
            //m_needToUpdateUBO = true;
            for(auto b : m_needToUpdateModel) b = true;

    m_texturePosition = position;
    m_textureExtent = extent;
}

AssetTypeID Sprite::getTexture()
{
    return m_texture;
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
    m_drawCommandBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VInstance::commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = (uint32_t) m_drawCommandBuffers.size();


    if (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_drawCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");
}

void Sprite::createVertexBuffer()
{
    VkDevice device = VInstance::device();

    std::vector<Vertex2D> vertices =
            {
                {glm::vec2(0,1), {1.0f, 0.0f, 0.0f}, m_texturePosition+glm::vec2(0,m_textureExtent.y)},
                {glm::vec2(0,0), {0.0f, 1.0f, 0.0f}, m_texturePosition},
                {glm::vec2(1,1), {0.0f, 0.0f, 1.0f}, m_texturePosition+m_textureExtent},
                {glm::vec2(1,0), {1.0f, 1.0f, 1.0f}, m_texturePosition+glm::vec2(m_textureExtent.x,0)}
            };

    VkDeviceSize bufferSize = sizeof(Vertex2D) * vertices.size();

    VBuffer stagingBuffer;
    VMemoryAllocator::allocBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  stagingBuffer);

    void* data;
    vkMapMemory(device, stagingBuffer.bufferMemory, stagingBuffer.offset, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBuffer.bufferMemory);

    VMemoryAllocator::allocBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    m_vertexBuffer);
    VMemoryAllocator::copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    VMemoryAllocator::freeBuffer(stagingBuffer);
}

void Sprite::updateModelUBO(DefaultRenderer *renderer, size_t currentFrame)
{
    ModelUBO modelUBO = {};
    modelUBO.model = glm::mat4(1.0);
    modelUBO.model = glm::translate(modelUBO.model, glm::vec3(m_position.x, m_position.y, 0.0));
    modelUBO.model = glm::scale(modelUBO.model, glm::vec3(m_size.x, m_size.y, 1.0));

    renderer->updateModelUBO(m_modelUBOIndex, &modelUBO,currentFrame);
    m_needToUpdateModel[currentFrame] = false;
}

VkCommandBuffer Sprite::getDrawCommandBuffer(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    if(m_needToCreateBuffers)
    {
        this->createAllBuffers();
        for(auto b : m_needToUpdateDrawCommandBuffer) b = true;
    }

    if(m_needToAllocModel[currentFrame])
    {
        if(!renderer->allocModelUBO(m_modelUBOIndex, currentFrame))
            return VK_NULL_HANDLE;

        m_needToAllocModel[currentFrame] = false;
    }

    if(m_needToUpdateModel[currentFrame])
        this->updateModelUBO(renderer, currentFrame);

    if(m_modelBufferVersion[currentFrame] != renderer->getModelUBOBufferVersion(currentFrame))
    {
        m_needToUpdateDrawCommandBuffer[currentFrame] = true;
        m_modelBufferVersion[currentFrame] = renderer->getModelUBOBufferVersion(currentFrame);
    }

    if(m_needToUpdateDrawCommandBuffer[currentFrame])
    {
        if(!this->recordDrawCommandBuffers(renderer, currentFrame, renderPass, subpass, framebuffer))
            return VK_NULL_HANDLE;
    }

    return m_drawCommandBuffers[currentFrame];

}

bool Sprite::recordDrawCommandBuffers(DefaultRenderer *renderer, size_t currentFrame,/*VkPipeline pipeline,*/ VkRenderPass renderPass,
                                      uint32_t subpass, VkFramebuffer framebuffer)
{
   /// for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
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

        if (vkBeginCommandBuffer(m_drawCommandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin recording command buffer");

        VkBuffer vertexBuffers[] = {m_vertexBuffer.buffer};
        VkDeviceSize offsets[] = {m_vertexBuffer.offset};

        if(!renderer->bindTexture(m_drawCommandBuffers[currentFrame], m_texture, currentFrame))
        {
            if (vkEndCommandBuffer(m_drawCommandBuffers[currentFrame]) != VK_SUCCESS)
                throw std::runtime_error("Failed to record command buffer");
            return (false);
        }

        renderer->bindAllUBOs(m_drawCommandBuffers[currentFrame],currentFrame,m_modelUBOIndex);

        vkCmdBindVertexBuffers(m_drawCommandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

        vkCmdDraw(m_drawCommandBuffers[currentFrame], 4, 1, 0, 0);

        if (vkEndCommandBuffer(m_drawCommandBuffers[currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");
    }

    m_needToUpdateDrawCommandBuffer[currentFrame] = false;
    return (true);
}

void Sprite::cleanup()
{
    VMemoryAllocator::freeBuffer(m_vertexBuffer);
}

}