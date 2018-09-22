#include "Valag/renderers/SceneRenderer.h"

#include "Valag/core/VApp.h"
#include "Valag/scene/IsoSpriteEntity.h"
#include "Valag/utils/Logger.h"

#include <sstream>

namespace vlg
{

const char *SceneRenderer::ISOSPRITE_VERTSHADERFILE = "isoSpriteShader.vert.spv";
const char *SceneRenderer::ISOSPRITE_FRAGSHADERFILE = "isoSpriteShader.frag.spv";
const char *SceneRenderer::AMBIENTLIGHTING_VERTSHADERFILE = "ambientLighting.vert.spv";
const char *SceneRenderer::AMBIENTLIGHTING_FRAGSHADERFILE = "ambientLighting.frag.spv";
const char *SceneRenderer::TONEMAPPING_VERTSHADERFILE = "toneMapping.vert.spv";
const char *SceneRenderer::TONEMAPPING_FRAGSHADERFILE = "toneMapping.frag.spv";


SceneRenderer::SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) :
    AbstractRenderer(targetWindow, name, order)
{
    this->init();
}

SceneRenderer::~SceneRenderer()
{
    this->cleanup();
}

void SceneRenderer::addToSpritesVbo(const InstanciedIsoSpriteDatum &datum)
{
    m_spritesVbos[m_curFrameIndex].push_back(datum);
}

bool SceneRenderer::recordToneMappingCmb(uint32_t imageIndex)
{
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_toneMappingPass, imageIndex, m_curFrameIndex);

        m_toneMappingPipeline.bind(cmb);
        vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_toneMappingPipeline.getLayout(),
                                0, 1, &m_hdrDescriptorSets[imageIndex], 0, NULL);
        vkCmdDraw(cmb, 3, 1, 0, 0);

    return m_renderGraph.endRecording(m_toneMappingPass);
}


bool SceneRenderer::recordPrimaryCmb(uint32_t imageIndex)
{
    size_t spritesVboSize = m_spritesVbos[m_curFrameIndex].uploadVBO();

    VkCommandBuffer cmb = m_renderGraph.startRecording(m_deferredPass, imageIndex, m_curFrameIndex);

        m_deferredPipeline.bind(cmb);

        VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                            VTexturesManager::instance()->getDescriptorSet(m_curFrameIndex) };

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_deferredPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);

        if(spritesVboSize != 0)
        {
            VBuffer vertexBuffer = m_spritesVbos[m_curFrameIndex].getBuffer();
            vkCmdBindVertexBuffers(cmb, 0, 1, &vertexBuffer.buffer, &vertexBuffer.offset);
            vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    return m_renderGraph.endRecording(m_deferredPass);
}


bool SceneRenderer::recordAmbientLightingCmb(uint32_t imageIndex)
{
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_ambientLightingPass, imageIndex, m_curFrameIndex);

        m_ambientLightingPipeline.bind(cmb);
        vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ambientLightingPipeline.getLayout(),
                                0, 1, &m_deferredDescriptorSets[imageIndex], 0, NULL);
        vkCmdDraw(cmb, 3, 1, 0, 0);

    return m_renderGraph.endRecording(m_ambientLightingPass);
}

bool SceneRenderer::init()
{
    m_renderView.setDepthFactor(1024*1024);
    m_renderView.setScreenOffset(glm::vec3(0.0f, 0.0f, 0.5f));

    m_spritesVbos.resize(m_targetWindow->getFramesCount(),
                         DynamicVBO<InstanciedIsoSpriteDatum>(1024));

    if(!this->createAttachments())
        return (false);

    if(!AbstractRenderer::init())
        return (false);

    for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        this->recordAmbientLightingCmb(i);
        this->recordToneMappingCmb(i);
    }

    return (true);
}

bool SceneRenderer::createRenderPass()
{
    if(!this->createDeferredRenderPass())
        return (false);
    if(!this->createAmbientLightingRenderPass())
        return (false);
    if(!this->createToneMappingRenderPass())
        return (false);

    m_renderGraph.connectRenderPasses(m_deferredPass, m_ambientLightingPass);
    m_renderGraph.connectRenderPasses(m_ambientLightingPass, m_toneMappingPass);

    return (true);
}

bool SceneRenderer::createDescriptorSetLayouts()
{
    VkDevice device = VInstance::device();

    std::vector<VkDescriptorSetLayoutBinding> deferredLayoutBindings;

    deferredLayoutBindings.resize(4);
    for(uint32_t i = 0 ; i < 4 ; ++i)
    {
        deferredLayoutBindings[i] = {};
        deferredLayoutBindings[i].binding = i;
        deferredLayoutBindings[i].descriptorCount = 1;
        deferredLayoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        deferredLayoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    VkDescriptorSetLayoutCreateInfo deferredLayoutInfo = {};
    deferredLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    deferredLayoutInfo.bindingCount = static_cast<uint32_t>(deferredLayoutBindings.size());
    deferredLayoutInfo.pBindings = deferredLayoutBindings.data();

    if(vkCreateDescriptorSetLayout(device, &deferredLayoutInfo, nullptr, &m_deferredDescriptorSetLayout) != VK_SUCCESS)
        return (false);

    VkDescriptorSetLayoutBinding hdrLayoutBinding = {};
    hdrLayoutBinding = {};
    hdrLayoutBinding.binding = 0;
    hdrLayoutBinding.descriptorCount = 1;
    hdrLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    hdrLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorSetLayoutCreateInfo hdrLayoutInfo = {};
    hdrLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    hdrLayoutInfo.bindingCount = 1;
    hdrLayoutInfo.pBindings = &hdrLayoutBinding;

    if(vkCreateDescriptorSetLayout(device, &hdrLayoutInfo, nullptr, &m_hdrDescriptorSetLayout) != VK_SUCCESS)
        return (false);

    return (true);
}

bool SceneRenderer::createGraphicsPipeline()
{
    if(!this->createDeferredPipeline())
        return (false);
    if(!this->createAmbientLightingPipeline())
        return (false);
    if(!this->createToneMappingPipeline())
        return (false);

    return (true);
}

bool SceneRenderer::createDescriptorPool()
{
    uint32_t imagesCount = static_cast<uint32_t>(m_targetWindow->getSwapchainSize());

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imagesCount});
    poolSizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imagesCount});

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();

    poolInfo.maxSets = imagesCount * poolSizes.size();

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool SceneRenderer::createDescriptorSets()
{
    VkDevice device = VInstance::device();
    uint32_t imagesCount = static_cast<uint32_t>(m_targetWindow->getSwapchainSize());

    std::vector<VkDescriptorSetLayout> layouts(imagesCount, m_deferredDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(imagesCount);
    allocInfo.pSetLayouts = layouts.data();

    m_deferredDescriptorSets.resize(imagesCount);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_deferredDescriptorSets.data()) != VK_SUCCESS)
        return (false);

    layouts = std::vector<VkDescriptorSetLayout>(imagesCount, m_hdrDescriptorSetLayout);
    m_hdrDescriptorSets.resize(imagesCount);
    allocInfo.pSetLayouts = layouts.data();
    if (vkAllocateDescriptorSets(device, &allocInfo,m_hdrDescriptorSets.data()) != VK_SUCCESS)
        return (false);

    for (size_t i = 0; i < imagesCount ; ++i)
    {

        std::array<VkWriteDescriptorSet,4+1> descriptorWrites;
        for(size_t j = 0 ; j < descriptorWrites.size() ; ++j)
        {
            descriptorWrites[j] = {};
            descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j].dstArrayElement = 0;
            descriptorWrites[j].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j].descriptorCount = 1;
        }

        std::array<VkDescriptorImageInfo,4> deferredImageInfos{};

        deferredImageInfos[0].imageView = m_albedoAttachments[i].view;
        deferredImageInfos[1].imageView = m_heightAttachments[i].view;
        deferredImageInfos[2].imageView = m_normalAttachments[i].view;
        deferredImageInfos[3].imageView = m_rmtAttachments[i].view;

        for(size_t j = 0 ; j < deferredImageInfos.size() ; ++j)
        {
            deferredImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            deferredImageInfos[j].sampler     = m_attachmentsSampler;

            descriptorWrites[j].dstSet = m_deferredDescriptorSets[i];
            descriptorWrites[j].dstBinding = j;
            descriptorWrites[j].pImageInfo = &deferredImageInfos[j];
        }

        VkDescriptorImageInfo hdrImageInfo = {};
        hdrImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        hdrImageInfo.sampler     = m_attachmentsSampler;
        hdrImageInfo.imageView   = m_hdrAttachements[i].view;

        descriptorWrites[4].dstSet = m_hdrDescriptorSets[i];
        descriptorWrites[4].dstBinding = 0;
        descriptorWrites[4].pImageInfo = &hdrImageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    return (true);
}

bool SceneRenderer::createAttachments()
{
    size_t imagesCount = m_targetWindow->getSwapchainSize();
    m_albedoAttachments.resize(imagesCount);
    m_heightAttachments.resize(imagesCount);
    m_normalAttachments.resize(imagesCount);
    m_rmtAttachments.resize(imagesCount);
    m_hdrAttachements.resize(imagesCount);

    uint32_t width = m_targetWindow->getSwapchainExtent().width;
    uint32_t height = m_targetWindow->getSwapchainExtent().height;

    for(size_t i = 0 ; i < imagesCount ; ++i)
    {
        if(!
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, m_albedoAttachments[i]) &
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_D32_SFLOAT,
                                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, m_heightAttachments[i]) &
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_normalAttachments[i]) &
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_rmtAttachments[i]) &
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SFLOAT,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_hdrAttachements[i])
        )
            return (false);

        /*VulkanHelpers::transitionImageLayout(m_albedoAttachments[i].image.vkImage, 0, VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VulkanHelpers::transitionImageLayout(m_heightAttachments[i].image.vkImage, 0, VK_FORMAT_D32_SFLOAT,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        VulkanHelpers::transitionImageLayout(m_normalAttachments[i].image.vkImage, 0, VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VulkanHelpers::transitionImageLayout(m_rmtAttachments[i].image.vkImage, 0, VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VulkanHelpers::transitionImageLayout(m_hdrAttachements[i].image.vkImage, 0, VK_FORMAT_R16G16B16A16_SFLOAT,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);*/
    }

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType       = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter   = VK_FILTER_NEAREST;
    samplerInfo.minFilter   = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode  = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias  = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.anisotropyEnable = VK_FALSE;

    return (vkCreateSampler(VInstance::device(), &samplerInfo, nullptr, &m_attachmentsSampler) == VK_SUCCESS);
}

bool SceneRenderer::createDeferredRenderPass()
{
    m_deferredPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        std::vector<VFramebufferAttachment> attachements = {m_albedoAttachments[i],
                                                            m_heightAttachments[i],
                                                            m_normalAttachments[i],
                                                            m_rmtAttachments[i]};
        m_renderGraph.setAttachments(m_deferredPass, i, attachements);
    }

    return (true);
}

bool SceneRenderer::createAmbientLightingRenderPass()
{
    m_ambientLightingPass = m_renderGraph.addRenderPass(0);
    for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        std::vector<VFramebufferAttachment> attachements = {m_hdrAttachements[i]};
        m_renderGraph.setAttachments(m_ambientLightingPass, i, attachements);
    }

    return (true);
}

bool SceneRenderer::createToneMappingRenderPass()
{
    m_toneMappingPass = m_renderGraph.addRenderPass(0);
    for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        std::vector<VFramebufferAttachment> attachements = {m_targetWindow->getSwapchainAttachments()[i],
                                                            m_targetWindow->getSwapchainDepthAttachments()[i]};
        m_renderGraph.setAttachments(m_toneMappingPass, i, attachements);
    }

    return (true);
}

bool SceneRenderer::createDeferredPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_FRAGSHADERFILE;

    m_deferredPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_deferredPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = InstanciedIsoSpriteDatum::getBindingDescription();
    auto attributeDescriptions = InstanciedIsoSpriteDatum::getAttributeDescriptions();
    m_deferredPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_deferredPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);

    m_deferredPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_deferredPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_deferredPipeline.attachDescriptorSetLayout(VTexturesManager::instance()->getDescriptorSetLayout());

    m_deferredPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER);

    return m_deferredPipeline.init(m_renderGraph.getVkRenderPass(m_deferredPass), 0, 3);
}


bool SceneRenderer::createAmbientLightingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << AMBIENTLIGHTING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << AMBIENTLIGHTING_FRAGSHADERFILE;

    m_ambientLightingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_ambientLightingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_ambientLightingPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_ambientLightingPipeline.attachDescriptorSetLayout(m_deferredDescriptorSetLayout);

    return m_ambientLightingPipeline.init(m_renderGraph.getVkRenderPass(m_ambientLightingPass), 0);
}

bool SceneRenderer::createToneMappingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << TONEMAPPING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << TONEMAPPING_FRAGSHADERFILE;

    m_toneMappingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_toneMappingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_toneMappingPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_toneMappingPipeline.setBlendMode(BlendMode_None);

    m_toneMappingPipeline.attachDescriptorSetLayout(m_hdrDescriptorSetLayout);

    return m_toneMappingPipeline.init(m_renderGraph.getVkRenderPass(m_toneMappingPass), 0);
}

void SceneRenderer::cleanup()
{
    VkDevice device = VInstance::device();

    m_spritesVbos.clear();

    vkDestroySampler(device, m_attachmentsSampler, nullptr);

    for(auto attachement : m_albedoAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_albedoAttachments.clear();

    for(auto attachement : m_heightAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_heightAttachments.clear();

    for(auto attachement : m_normalAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_normalAttachments.clear();

    for(auto attachement : m_rmtAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_rmtAttachments.clear();

    for(auto attachement : m_hdrAttachements)
        VulkanHelpers::destroyAttachment(attachement);
    m_hdrAttachements.clear();

    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, m_deferredDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_hdrDescriptorSetLayout, nullptr);

    m_deferredPipeline.destroy();
    m_ambientLightingPipeline.destroy();
    m_toneMappingPipeline.destroy();

    AbstractRenderer::cleanup();
}


}
