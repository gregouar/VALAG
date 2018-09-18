#include "Valag/gfx/Sprite.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Valag/renderers/DefaultRenderer.h"
#include "Valag/assets/AssetHandler.h"
#include "Valag/assets/TextureAsset.h"

namespace vlg
{


Sprite::Sprite() :
    m_size({0.0f,0.0f}),
    m_position({0.0f,0.0f,0.0f}),
    m_color({1.0f,1.0f,1.0f,1.0f}),
    m_texture(0),
    m_texturePosition({0.0f,0.0f}),
    m_textureExtent({1.0f,1.0f}),
    m_preventDrawing(false)
{
    ///I should probably move this to checkUpdates somehow
    m_needToAllocModel      = std::vector<bool> (s_framesCount, true);
    m_needToUploadModelUBO  = std::vector<bool> (s_framesCount, true);
    m_modelBufferVersion    = std::vector<size_t> (s_framesCount, 0);
    m_texDescSetVersion     = std::vector<size_t> (s_framesCount, 0);
    m_needToCheckLoading    = std::vector<bool> (s_framesCount, false);
    m_modelUBOIndex.resize(s_framesCount);


    this->updateModelUBO();
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
        {
            m_size = size;
            this->updateModelUBO();
        }
    }
}

void Sprite::setPosition(glm::vec2 position)
{
    this->setPosition(glm::vec3(position.x, position.y, 0.0));
}

void Sprite::setPosition(glm::vec3 position)
{
    if(m_position != position)
    {
        m_position = position;
        this->updateModelUBO();
    }
}

void Sprite::setColor(glm::vec4 color)
{
    if(m_color != color)
    {
        m_color = color;
        this->updateModelUBO();
    }
}

void Sprite::setTexture(AssetTypeID textureID)
{
    if(textureID != m_texture)
    {
        sendNotification(Notification_TextureIsAboutToChange);

        m_texture = textureID;
        if(textureID != 0 && !TexturesHandler::instance()->getAsset(textureID)->isLoaded())
            for(auto b : m_needToCheckLoading) b = true;


        this->sendNotification(Notification_TextureChanged);
    }
}

void Sprite::setTextureRect(glm::vec2 position, glm::vec2 extent)
{
    if(m_texturePosition != position || m_textureExtent != extent)
    {
        m_texturePosition   = position;
        m_textureExtent     = extent;
        this->updateModelUBO();
    }
}

SpriteModelUBO Sprite::getModelUBO()
{
    return m_modelUBO;
}

AssetTypeID Sprite::getTexture()
{
    return m_texture;
}


/*void Sprite::createVertexBuffer()
{
    std::vector<Vertex2D> vertices =
            {
                {glm::vec2(0,1),  m_texturePosition+glm::vec2(0,m_textureExtent.y)},
                {glm::vec2(0,0),  m_texturePosition},
                {glm::vec2(1,1),  m_texturePosition+m_textureExtent},
                {glm::vec2(1,0),  m_texturePosition+glm::vec2(m_textureExtent.x,0)}
            };

    VkDeviceSize bufferSize = sizeof(Vertex2D) * vertices.size();

    VBuffer stagingBuffer;
    VBuffersAllocator::allocBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  stagingBuffer);

    VBuffersAllocator::writeBuffer(stagingBuffer,vertices.data(), (size_t) bufferSize);

    VBuffersAllocator::allocBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    m_vertexBuffer);
    VBuffersAllocator::copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    VBuffersAllocator::freeBuffer(stagingBuffer);

    m_needToCreateVertexBuffer = false;
}*/

void Sprite::updateModelUBO()
{
    m_modelUBO.model = glm::mat4(1.0);
    m_modelUBO.model = glm::translate(m_modelUBO.model, glm::vec3(m_position.x, m_position.y, m_position.z));
    m_modelUBO.model = glm::scale(m_modelUBO.model, glm::vec3(m_size.x, m_size.y, 1.0));
    m_modelUBO.color = m_color;
    m_modelUBO.texPos = m_texturePosition;
    m_modelUBO.texExt = m_textureExtent;
    for(auto b : m_needToUploadModelUBO) b = true;
}

void Sprite::uploadModelUBO(size_t frameIndex)
{
    /*SpriteModelUBO modelUBO = {};
    modelUBO.model = glm::mat4(1.0);
    modelUBO.model = glm::translate(modelUBO.model, glm::vec3(m_position.x, m_position.y, m_position.z));
    modelUBO.model = glm::scale(modelUBO.model, glm::vec3(m_size.x, m_size.y, 1.0));
    modelUBO.color = m_color;
    modelUBO.texPos = m_texturePosition;
    modelUBO.texExt = m_textureExtent;*/

    s_modelUBO.updateObject(frameIndex, m_modelUBOIndex[frameIndex], &m_modelUBO);

    m_needToUploadModelUBO[frameIndex] = false;
}

bool Sprite::checkUpdates(size_t frameIndex)
{
    m_preventDrawing = false;
    bool needToUpdateDrawCmb = false;

    if(m_needToAllocModel[frameIndex])
    {
        if(!s_modelUBO.allocObject(frameIndex, m_modelUBOIndex[frameIndex]))
        {
            m_preventDrawing = true;
            return (false);
        }
        m_needToAllocModel[frameIndex] = false;
        needToUpdateDrawCmb = true;
    }

    if(m_modelBufferVersion[frameIndex] != s_modelUBO.getBufferVersion(frameIndex))
    {
        needToUpdateDrawCmb = true;
        m_modelBufferVersion[frameIndex] = s_modelUBO.getBufferVersion(frameIndex);
    }

    if(m_needToUploadModelUBO[frameIndex])
        this->uploadModelUBO(frameIndex);

    if(m_texDescSetVersion[frameIndex] != VTexturesManager::descriptorSetVersion(frameIndex))
    {
        needToUpdateDrawCmb = true;
        m_texDescSetVersion[frameIndex] = VTexturesManager::descriptorSetVersion(frameIndex);
    }

    if(m_needToCheckLoading[frameIndex] && TexturesHandler::instance()->getAsset(m_texture)->isLoaded())
    {
        needToUpdateDrawCmb = true;
        m_needToCheckLoading[frameIndex] = false;
    }

    return needToUpdateDrawCmb;
}

VkCommandBuffer Sprite::getDrawCommandBuffer(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    if(this->checkUpdates(frameIndex))
        this->askToUpdateDrawCmb(frameIndex);
       // m_needToUpdateDrawCmb[frameIndex] = true;

    if(m_preventDrawing)
        return (VK_NULL_HANDLE);

    return Drawable::getDrawCommandBuffer(renderer, frameIndex, renderPass, subpass, framebuffer);
}


bool Sprite::recordDrawCmbContent(VkCommandBuffer &commandBuffer,DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                  uint32_t subpass, VkFramebuffer framebuffer)
{
    if(m_preventDrawing)
        return (false);

    if(m_texture != 0 && !TexturesHandler::instance()->getAsset(m_texture)->isLoaded())
        return (false);

    if(!renderer->bindTexture(commandBuffer, m_texture, frameIndex))
        return (false);

    renderer->bindModelDescriptorSet(frameIndex, commandBuffer, s_modelUBO, m_modelUBOIndex[frameIndex]);

    vkCmdDraw(commandBuffer, 4, 1, 0, 0);

    return (true);
}

bool Sprite::recordDrawCommandBuffers(DefaultRenderer *renderer, size_t frameIndex,/*VkPipeline pipeline,*/ VkRenderPass renderPass,
                                      uint32_t subpass, VkFramebuffer framebuffer)
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

    if (vkBeginCommandBuffer(m_drawCommandBuffers[frameIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer");

    renderer->bindPipeline(m_drawCommandBuffers[frameIndex],frameIndex);

    if(!this->recordDrawCmbContent(m_drawCommandBuffers[frameIndex], renderer, frameIndex, renderPass, subpass, framebuffer))
    {
        if (vkEndCommandBuffer(m_drawCommandBuffers[frameIndex]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");
        return (false);
    }

    if (vkEndCommandBuffer(m_drawCommandBuffers[frameIndex]) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");

    return (true);
}

void Sprite::cleanup()
{
    for(size_t i = 0 ; i < m_modelUBOIndex.size() ; ++i)
        s_modelUBO.freeObject(i,m_modelUBOIndex[i]);
}

/// Static ///

DynamicUBODescriptor Sprite::s_modelUBO = DynamicUBODescriptor(sizeof(SpriteModelUBO),1024); //Chunk size
size_t Sprite::s_framesCount = 1;

bool Sprite::initRendering(size_t framesCount)
{
    s_framesCount = framesCount;
    return s_modelUBO.init(framesCount);
}

void Sprite::updateRendering(size_t frameIndex)
{
    s_modelUBO.update(frameIndex);
}

void Sprite::cleanupRendering()
{
    s_modelUBO.cleanup();
}

VkDescriptorSetLayout Sprite::getModelDescriptorSetLayout()
{
    return s_modelUBO.getDescriptorSetLayout();
}


}
