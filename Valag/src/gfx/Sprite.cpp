#include "Valag/gfx/Sprite.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Valag/core/VApp.h"
#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/gfx/DefaultRenderer.h"
#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"

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
    m_needToAllocModel      = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, true);
    m_needToUpdateModel     = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, true);
    m_modelBufferVersion    = std::vector<size_t> (VApp::MAX_FRAMES_IN_FLIGHT, 0);
    m_texDescSetVersion     = std::vector<size_t> (VApp::MAX_FRAMES_IN_FLIGHT, 0);
    m_needToCheckLoading    = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, false);
    m_modelUBOIndex.resize(VApp::MAX_FRAMES_IN_FLIGHT);
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
            for(auto b : m_needToUpdateModel) b = true;
        m_size = size;
    }
}

void Sprite::setPosition(glm::vec2 position)
{
    this->setPosition(glm::vec3(position.x, position.y, 0.0));
}

void Sprite::setPosition(glm::vec3 position)
{
    if(m_position != position)
            for(auto b : m_needToUpdateModel) b = true;
    m_position = position;
}

void Sprite::setColor(glm::vec4 color)
{
    if(m_color != color)
        for(auto b : m_needToUpdateModel) b = true;
    m_color = color;
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
    if(m_texturePosition != position || extent != m_textureExtent)
        for(auto b : m_needToUpdateModel) b = true;
            //for(auto b : m_needToUpdateVertexBuffer) b = true;

    m_texturePosition = position;
    m_textureExtent = extent;
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

void Sprite::updateModelUBO(DefaultRenderer *renderer, size_t frameIndex)
{
    SpriteModelUBO modelUBO = {};
    modelUBO.model = glm::mat4(1.0);
    modelUBO.model = glm::translate(modelUBO.model, glm::vec3(m_position.x, m_position.y, m_position.z));
    modelUBO.model = glm::scale(modelUBO.model, glm::vec3(m_size.x, m_size.y, 1.0));
    modelUBO.color = m_color;
    modelUBO.texPos = m_texturePosition;
    modelUBO.texExt = m_textureExtent;

    //renderer->updateModelUBO(m_modelUBOIndex, &modelUBO,frameIndex);
    //s_modelBuffers[frameIndex]->updateObject(m_modelUBOIndex[frameIndex], &modelUBO);
    s_modelUBO.updateObject(frameIndex, m_modelUBOIndex[frameIndex], &modelUBO);

    m_needToUpdateModel[frameIndex] = false;
}

bool Sprite::checkUpdates(DefaultRenderer *renderer, size_t frameIndex)
{
    m_preventDrawing = false;
    bool needToUpdateDrawCMB = false;

    if(m_needToAllocModel[frameIndex])
    {
        /*if(s_modelBuffers[frameIndex]->isFull())
        {
            s_needToExpandModelBuffers[frameIndex] = true;
            m_preventDrawing = true;
            return (false);
        }

        s_modelBuffers[frameIndex]->allocObject(m_modelUBOIndex[frameIndex]);
        m_needToAllocModel[frameIndex] = false;
        needToUpdateDrawCMB = true;*/

        if(!s_modelUBO.allocObject(frameIndex, m_modelUBOIndex[frameIndex]))
        {
            m_preventDrawing = true;
            return (false);
        }
        m_needToAllocModel[frameIndex] = false;
        needToUpdateDrawCMB = true;
    }

    if(m_modelBufferVersion[frameIndex] != s_modelUBO.getBufferVersion(frameIndex))
    {
        needToUpdateDrawCMB = true;
        m_modelBufferVersion[frameIndex] = s_modelUBO.getBufferVersion(frameIndex);
    }

    if(m_needToUpdateModel[frameIndex])
        this->updateModelUBO(renderer, frameIndex);

    if(m_texDescSetVersion[frameIndex] != renderer->getTextureArrayDescSetVersion(frameIndex))
    {
        needToUpdateDrawCMB = true;
        m_texDescSetVersion[frameIndex] = renderer->getTextureArrayDescSetVersion(frameIndex);
    }

    if(m_needToCheckLoading[frameIndex] && TexturesHandler::instance()->getAsset(m_texture)->isLoaded())
    {
        needToUpdateDrawCMB = true;
        m_needToCheckLoading[frameIndex] = false;
    }

    return needToUpdateDrawCMB;
}

VkCommandBuffer Sprite::getDrawCommandBuffer(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer)
{
    if(this->checkUpdates(renderer, frameIndex))
        m_needToUpdateDrawCMB[frameIndex] = true;

    if(m_preventDrawing)
        return (VK_NULL_HANDLE);

    return Drawable::getDrawCommandBuffer(renderer, frameIndex, renderPass, subpass, framebuffer);
}


bool Sprite::recordDrawCMBContent(VkCommandBuffer &commandBuffer,DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                  uint32_t subpass, VkFramebuffer framebuffer)
{
    if(m_preventDrawing)
        return (false);

    if(m_texture != 0 && !TexturesHandler::instance()->getAsset(m_texture)->isLoaded())
        return (false);

    if(!renderer->bindTexture(commandBuffer, m_texture, frameIndex))
        return (false);

    //uint32_t dynamicOffset = s_modelBuffers[frameIndex]->getDynamicOffset(m_modelUBOIndex[frameIndex]);
    //renderer->bindModelDescriptorSet(commandBuffer, s_modelDescriptorSets[frameIndex], dynamicOffset);
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

    renderer->bindDefaultPipeline(m_drawCommandBuffers[frameIndex],frameIndex);

    if(!this->recordDrawCMBContent(m_drawCommandBuffers[frameIndex], renderer, frameIndex, renderPass, subpass, framebuffer))
    {
        if (vkEndCommandBuffer(m_drawCommandBuffers[frameIndex]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");
        return (false);
    }

    if (vkEndCommandBuffer(m_drawCommandBuffers[frameIndex]) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");

    m_needToUpdateDrawCMB[frameIndex] = false;
    return (true);
}

void Sprite::cleanup()
{
   // VBuffersAllocator::freeBuffer(m_vertexBuffer);
  // for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)

    //for(auto modelBuffer : s_modelBuffers)
    for(size_t i = 0 ; i < m_modelUBOIndex.size() ; ++i)
        s_modelUBO.freeObject(i,m_modelUBOIndex[i]);
}

/// Static ///

DynamicUBODescriptor Sprite::s_modelUBO = DynamicUBODescriptor(sizeof(SpriteModelUBO),1024); //Chunk size

bool Sprite::initSpriteRendering()
{
    return s_modelUBO.init();
}

void Sprite::updateSpriteRendering(size_t frameIndex)
{
    s_modelUBO.update(frameIndex);
}

void Sprite::cleanupSpriteRendering()
{
    s_modelUBO.cleanup();
}

VkDescriptorSetLayout Sprite::getModelDescriptorSetLayout()
{
    return s_modelUBO.getDescriptorSetLayout();
}

/*const size_t Sprite::MODEL_UBO_CHUNKSIZE = 1024;

std::vector<bool>               Sprite::s_needToExpandModelBuffers;
std::vector<DynamicUBO*>        Sprite::s_modelBuffers;
VkDescriptorSetLayout           Sprite::s_modelDescriptorSetLayout;
VkDescriptorPool                Sprite::s_descriptorPool;
std::vector<VkDescriptorSet>    Sprite::s_modelDescriptorSets;

bool Sprite::initSpriteRendering()
{
    if(!Sprite::createDescriptorSetLayouts())
        return (false);

    s_needToExpandModelBuffers = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, false);
    s_modelBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
        s_modelBuffers[i] = new DynamicUBO(sizeof(SpriteModelUBO),Sprite::MODEL_UBO_CHUNKSIZE);

    if(!Sprite::createDescriptorPool())
        return (false);

    if(!Sprite::createDescriptorSets())
        return (false);

    return (true);
}

void Sprite::updateSpriteRendering(size_t frameIndex)
{
    if(s_needToExpandModelBuffers[frameIndex])
    {
        s_modelBuffers[frameIndex]->expandBuffers();
        Sprite::updateModelDescriptorSets(frameIndex);
        s_needToExpandModelBuffers[frameIndex] = false;
    }
}

void Sprite::cleanupSpriteRendering()
{
    auto device = VInstance::device();

    vkDestroyDescriptorPool(device,s_descriptorPool,nullptr);

    for(auto modelBuffer : s_modelBuffers)
        delete modelBuffer;
    s_modelBuffers.clear();

    vkDestroyDescriptorSetLayout(device, s_modelDescriptorSetLayout, nullptr);
}

bool Sprite::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if(vkCreateDescriptorSetLayout(VInstance::device(), &layoutInfo, nullptr, &s_modelDescriptorSetLayout) != VK_SUCCESS)
        return (false);

    return (true);
}

bool Sprite::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT);

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &s_descriptorPool) == VK_SUCCESS);
}

bool Sprite::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(VApp::MAX_FRAMES_IN_FLIGHT, s_modelDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = s_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    s_modelDescriptorSets.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(VInstance::device(), &allocInfo,s_modelDescriptorSets.data()) != VK_SUCCESS)
        return (false);

    for (size_t i = 0; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
        Sprite::updateModelDescriptorSets(i);

    return (true);
}


void Sprite::updateModelDescriptorSets(size_t frameIndex)
{
    VkDevice device = VInstance::device();

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = s_modelBuffers[frameIndex]->getBuffer().buffer;
    bufferInfo.offset = s_modelBuffers[frameIndex]->getBuffer().offset;
    bufferInfo.range = sizeof(SpriteModelUBO);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = s_modelDescriptorSets[frameIndex];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}


VkDescriptorSetLayout Sprite::getModelDescriptorSetLayout()
{
    return s_modelDescriptorSetLayout;
}*/


}
