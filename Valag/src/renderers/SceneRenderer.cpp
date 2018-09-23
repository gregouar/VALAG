#include "Valag/renderers/SceneRenderer.h"

#include "Valag/core/VApp.h"
#include "Valag/scene/IsoSpriteEntity.h"
#include "Valag/utils/Logger.h"

#include <sstream>

namespace vlg
{

const char *SceneRenderer::ISOSPRITE_DEFERRED_VERTSHADERFILE = "isoSpriteShader.vert.spv";
const char *SceneRenderer::ISOSPRITE_DEFERRED_FRAGSHADERFILE = "isoSpriteShader.frag.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADETECT_VERTSHADERFILE = "isoSpriteAlphaDetection.vert.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADETECT_FRAGSHADERFILE = "isoSpriteAlphaDetection.frag.spv";
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
        //vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_toneMappingPipeline.getLayout(),
          //                      0, 1, &m_hdrDescriptorSets[imageIndex], 0, NULL);
        VkDescriptorSet descSets[] = {m_renderGraph.getDescriptorSet(m_toneMappingPass,imageIndex)};
        vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_toneMappingPipeline.getLayout(),
                                0, 1, descSets, 0, NULL);
        vkCmdDraw(cmb, 3, 1, 0, 0);

    return m_renderGraph.endRecording(m_toneMappingPass);
}


bool SceneRenderer::recordPrimaryCmb(uint32_t imageIndex)
{
    size_t spritesVboSize = m_spritesVbos[m_curFrameIndex].uploadVBO();
    VBuffer vertexBuffer = m_spritesVbos[m_curFrameIndex].getBuffer();

    VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                        VTexturesManager::instance()->getDescriptorSet(m_curFrameIndex) };

    VkCommandBuffer cmb = m_renderGraph.startRecording(m_deferredPass, imageIndex, m_curFrameIndex);


        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_deferredPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);

        if(spritesVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &vertexBuffer.buffer, &vertexBuffer.offset);

            m_deferredPipeline.bind(cmb);

            vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_deferredPass))
        return (false);

    cmb = m_renderGraph.startRecording(m_alphaDetectPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_alphaDetectPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);

        if(spritesVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &vertexBuffer.buffer, &vertexBuffer.offset);

            m_alphaDetectPipeline.bind(cmb);

            vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_alphaDetectPass))
        return (false);

    return (true);
}


bool SceneRenderer::recordAmbientLightingCmb(uint32_t imageIndex)
{
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_ambientLightingPass, imageIndex, m_curFrameIndex);

        m_ambientLightingPipeline.bind(cmb);
        //vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ambientLightingPipeline.getLayout(),
          //                      0, 1, &m_deferredDescriptorSets[imageIndex], 0, NULL);

        VkDescriptorSet descSets[] = {m_renderGraph.getDescriptorSet(m_ambientLightingPass,imageIndex)};
        vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ambientLightingPipeline.getLayout(),
                                0, 1, descSets, 0, NULL);
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
    if(!this->createAlphaDetectRenderPass())
        return (false);
    if(!this->createAmbientLightingRenderPass())
        return (false);
    if(!this->createToneMappingRenderPass())
        return (false);

    m_renderGraph.connectRenderPasses(m_deferredPass, m_ambientLightingPass);
    m_renderGraph.connectRenderPasses(m_deferredPass, m_alphaDetectPass);
    m_renderGraph.connectRenderPasses(m_ambientLightingPass, m_toneMappingPass);

    return (true);
}

/*bool SceneRenderer::createDescriptorSetLayouts()
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
}*/

bool SceneRenderer::createGraphicsPipeline()
{
    if(!this->createDeferredPipeline())
        return (false);
    if(!this->createAlphaDetectPipeline())
        return (false);
    if(!this->createAmbientLightingPipeline())
        return (false);
    if(!this->createToneMappingPipeline())
        return (false);

    return (true);
}

/*bool SceneRenderer::createDescriptorPool()
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

        deferredImageInfos[0].imageView = m_albedoAttachments[0][i].view;
        deferredImageInfos[1].imageView = m_positionAttachments[0][i].view;
        deferredImageInfos[2].imageView = m_normalAttachments[0][i].view;
        deferredImageInfos[3].imageView = m_rmtAttachments[0][i].view;

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
        hdrImageInfo.imageView   = m_hdrAttachements[0][i].view;

        descriptorWrites[4].dstSet = m_hdrDescriptorSets[i];
        descriptorWrites[4].dstBinding = 0;
        descriptorWrites[4].pImageInfo = &hdrImageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    return (true);
}*/

bool SceneRenderer::createAttachments()
{
    size_t imagesCount  = m_targetWindow->getSwapchainSize();
    uint32_t width      = m_targetWindow->getSwapchainExtent().width;
    uint32_t height     = m_targetWindow->getSwapchainExtent().height;

    m_deferredDepthAttachments.resize(imagesCount);
    m_alphaDetectAttachments.resize(imagesCount);

    for(size_t a = 0 ; a < NBR_ALPHA_LAYERS ; ++a)
    {
        m_albedoAttachments[a].resize(imagesCount);
        m_positionAttachments[a].resize(imagesCount);
        m_normalAttachments[a].resize(imagesCount);
        m_rmtAttachments[a].resize(imagesCount);
        m_hdrAttachements[a].resize(imagesCount);

        for(size_t i = 0 ; i < imagesCount ; ++i)
        {
            if(a == 0)
            {
                if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_D32_SFLOAT,
                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, m_deferredDepthAttachments[i]))
                    return (false);

                if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8_UINT,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_alphaDetectAttachments[i]))
                    return (false);
            }

            if(!
                VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_albedoAttachments[a][i]) &
                VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_positionAttachments[a][i]) &
                VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_normalAttachments[a][i]) &
                VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_rmtAttachments[a][i]) &
                VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_hdrAttachements[a][i])
            )
            return (false);
        }
    }

    return (true);


    /*VkSamplerCreateInfo samplerInfo = {};
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

    return (vkCreateSampler(VInstance::device(), &samplerInfo, nullptr, &m_attachmentsSampler) == VK_SUCCESS);*/
}

bool SceneRenderer::createDeferredRenderPass()
{
    m_deferredPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    /*for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        std::vector<VFramebufferAttachment> attachements = {m_albedoAttachments[0][i],
                                                            m_positionAttachments[0][i],
                                                            m_normalAttachments[0][i],
                                                            m_rmtAttachments[0][i],
                                                            m_deferredDepthAttachments[i]};
        m_renderGraph.setAttachments(m_deferredPass, i, attachements);
    }*/

    m_renderGraph.addNewAttachments(m_deferredPass, m_albedoAttachments[0]);
    m_renderGraph.addNewAttachments(m_deferredPass, m_positionAttachments[0]);
    m_renderGraph.addNewAttachments(m_deferredPass, m_normalAttachments[0]);
    m_renderGraph.addNewAttachments(m_deferredPass, m_rmtAttachments[0]);
    m_renderGraph.addNewAttachments(m_deferredPass, m_deferredDepthAttachments);

    return (true);
}

bool SceneRenderer::createAlphaDetectRenderPass()
{
    m_alphaDetectPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    /*for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        std::vector<VFramebufferAttachment> attachements = {m_alphaDetectAttachments[i],
                                                            m_deferredDepthAttachments[i]};
        m_renderGraph.setAttachments(m_alphaDetectPass, i, attachements);
    }*/

    m_renderGraph.addNewAttachments(m_alphaDetectPass, m_alphaDetectAttachments, VK_ATTACHMENT_STORE_OP_STORE);
    m_renderGraph.transferAttachmentsToAttachments(m_deferredPass, m_alphaDetectPass, 4);
    //m_renderGraph.addNewAttachments(m_alphaDetectPass, m_deferredDepthAttachments, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_LOAD);

    return (true);
}

bool SceneRenderer::createAmbientLightingRenderPass()
{
    m_ambientLightingPass = m_renderGraph.addRenderPass(0);
    /*for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        std::vector<VFramebufferAttachment> attachements = {m_hdrAttachements[0][i]};
        m_renderGraph.setAttachments(m_ambientLightingPass, i, attachements);
    }*/
    m_renderGraph.addNewAttachments(m_ambientLightingPass, m_hdrAttachements[0]);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 3);

    return (true);
}

bool SceneRenderer::createToneMappingRenderPass()
{
    m_toneMappingPass = m_renderGraph.addRenderPass(0);
    /*for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        std::vector<VFramebufferAttachment> attachements = {m_targetWindow->getSwapchainAttachments()[i],
                                                            m_targetWindow->getSwapchainDepthAttachments()[i]};
        m_renderGraph.setAttachments(m_toneMappingPass, i, attachements);
    }*/
    m_renderGraph.addNewAttachments(m_toneMappingPass, m_targetWindow->getSwapchainAttachments(), VK_ATTACHMENT_STORE_OP_STORE);
    m_renderGraph.addNewAttachments(m_toneMappingPass, m_targetWindow->getSwapchainDepthAttachments(), VK_ATTACHMENT_STORE_OP_STORE);
    m_renderGraph.transferAttachmentsToUniforms(m_ambientLightingPass, m_toneMappingPass, 0);

    return (true);
}

bool SceneRenderer::createDeferredPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_DEFERRED_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_DEFERRED_FRAGSHADERFILE;

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

    return m_deferredPipeline.init(m_renderGraph.getVkRenderPass(m_deferredPass), 0, 4);
}

bool SceneRenderer::createAlphaDetectPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_ALPHADETECT_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_ALPHADETECT_FRAGSHADERFILE;

    m_alphaDetectPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_alphaDetectPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = InstanciedIsoSpriteDatum::getBindingDescription();
    auto attributeDescriptions = InstanciedIsoSpriteDatum::getAttributeDescriptions();
    m_alphaDetectPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_alphaDetectPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);

    m_alphaDetectPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_alphaDetectPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_alphaDetectPipeline.attachDescriptorSetLayout(VTexturesManager::instance()->getDescriptorSetLayout());

    m_alphaDetectPipeline.setDepthTest(false, true, VK_COMPARE_OP_GREATER);

    return m_alphaDetectPipeline.init(m_renderGraph.getVkRenderPass(m_alphaDetectPass), 0, 1);
}


bool SceneRenderer::createAmbientLightingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << AMBIENTLIGHTING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << AMBIENTLIGHTING_FRAGSHADERFILE;

    m_ambientLightingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_ambientLightingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_ambientLightingPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    //m_ambientLightingPipeline.attachDescriptorSetLayout(m_deferredDescriptorSetLayout);
    m_ambientLightingPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_ambientLightingPass));

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

    //m_toneMappingPipeline.attachDescriptorSetLayout(m_hdrDescriptorSetLayout);
    m_toneMappingPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_toneMappingPass));

    return m_toneMappingPipeline.init(m_renderGraph.getVkRenderPass(m_toneMappingPass), 0);
}

void SceneRenderer::cleanup()
{
    m_spritesVbos.clear();

    //vkDestroySampler(device, m_attachmentsSampler, nullptr);

    for(auto attachement : m_deferredDepthAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_deferredDepthAttachments.clear();

    for(auto attachement : m_alphaDetectAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_alphaDetectAttachments.clear();

    for(size_t a = 0 ; a < NBR_ALPHA_LAYERS ; ++a)
    {
        for(auto attachement : m_albedoAttachments[a])
            VulkanHelpers::destroyAttachment(attachement);
        m_albedoAttachments[a].clear();

        for(auto attachement : m_positionAttachments[a])
            VulkanHelpers::destroyAttachment(attachement);
        m_positionAttachments[a].clear();

        for(auto attachement : m_normalAttachments[a])
            VulkanHelpers::destroyAttachment(attachement);
        m_normalAttachments[a].clear();

        for(auto attachement : m_rmtAttachments[a])
            VulkanHelpers::destroyAttachment(attachement);
        m_rmtAttachments[a].clear();

        for(auto attachement : m_hdrAttachements[a])
            VulkanHelpers::destroyAttachment(attachement);
        m_hdrAttachements[a].clear();
    }

    m_deferredPipeline.destroy();
    m_ambientLightingPipeline.destroy();
    m_toneMappingPipeline.destroy();

    AbstractRenderer::cleanup();
}


}
