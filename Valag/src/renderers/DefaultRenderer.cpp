#include "Valag/renderers/DefaultRenderer.h"

#include <sstream>

#include "Valag/utils/Profiler.h"
#include "Valag/utils/Logger.h"
#include "Valag/core/Config.h"
#include "Valag/core/VApp.h"

#include "Valag/assets/AssetHandler.h"
#include "Valag/assets/TextureAsset.h"
#include "Valag/gfx/Sprite.h"

#include "Valag/vulkanImpl/VulkanHelpers.h"


namespace vlg
{

const char *DefaultRenderer::DEFAULT_VERTSHADERFILE = "defaultShader.vert.spv";
const char *DefaultRenderer::DEFAULT_FRAGSHADERFILE = "defaultShader.frag.spv";

const float DefaultRenderer::DEPTH_SCALING_FACTOR = 1024*1024;

DefaultRenderer::DefaultRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) : AbstractRenderer(targetWindow, name, order)
{
    this->init();
}

DefaultRenderer::~DefaultRenderer()
{
    this->cleanup();
}

void DefaultRenderer::update(size_t frameIndex)
{
    Sprite::updateRendering(frameIndex);
    AbstractRenderer::update(frameIndex);
}


void DefaultRenderer::draw(Drawable *drawable)
{
    VkCommandBuffer commandBuffer = drawable->getDrawCommandBuffer(this,m_curFrameIndex, m_renderGraph.getRenderPass(m_defaultPass)->getVkRenderPass(), 0);
    if(commandBuffer != VK_NULL_HANDLE)
        m_activeSecondaryCommandBuffers.push_back(commandBuffer);
}

bool DefaultRenderer::recordPrimaryCmb(uint32_t imageIndex)
{
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_defaultPass, imageIndex, m_curFrameIndex, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        if(!m_activeSecondaryCommandBuffers.empty())
            vkCmdExecuteCommands(cmb, (uint32_t) m_activeSecondaryCommandBuffers.size(), m_activeSecondaryCommandBuffers.data());
        m_activeSecondaryCommandBuffers.clear();

    return m_renderGraph.endRecording(m_defaultPass);
}

bool DefaultRenderer::bindTexture(VkCommandBuffer &commandBuffer, AssetTypeId textureId, size_t frameIndex)
{
    TextureAsset *texAsset = TexturesHandler::instance()->getAsset(textureId);

    int pc[] = {0,0};

    if(texAsset != nullptr && texAsset->isLoaded())
    {
        VTexture vtexture = texAsset->getVTexture();
        pc[0] = vtexture.getTextureId();
        pc[1] = vtexture.getTextureLayer();
    }

    vkCmdPushConstants(commandBuffer, m_pipeline.getLayout(),
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int)*2, (void*)pc);

    return (true);
}

void DefaultRenderer::bindPipeline(VkCommandBuffer &commandBuffer, size_t frameIndex)
{
    m_pipeline.bind(commandBuffer);

    VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(frameIndex),
                                        VTexturesManager::descriptorSet(frameIndex) };

    vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline.getLayout(),0,2, descriptorSets, 0, nullptr);
}

void DefaultRenderer::bindModelDescriptorSet(size_t frameIndex, VkCommandBuffer &commandBuffer, DynamicUBODescriptor &uboDesc, size_t index)
{
    uint32_t dynamicOffset  = uboDesc.getDynamicOffset(frameIndex, index);
    auto descSet            = uboDesc.getDescriptorSet(frameIndex);
    vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline.getLayout(),2,1, &descSet, 1, &dynamicOffset);
}

bool DefaultRenderer::init()
{
    Sprite::initRendering(m_targetWindow->getFramesCount());

    m_renderView.setDepthFactor(DEPTH_SCALING_FACTOR);
    m_renderView.setScreenOffset(glm::vec3(-1.0f, -1.0f, 0.5f));

    return AbstractRenderer::init();
}

bool DefaultRenderer::createDescriptorSetLayouts()
{
    return (true);
}

bool DefaultRenderer::createGraphicsPipeline()
{

    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << DEFAULT_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << DEFAULT_FRAGSHADERFILE;

    m_pipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_pipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_pipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);

    m_pipeline.setStaticExtent(m_targetWindow->getSwapchainExtent());
    m_pipeline.setBlendMode(BlendMode_Alpha);

    m_pipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_pipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());
    m_pipeline.attachDescriptorSetLayout(Sprite::getModelDescriptorSetLayout());

    m_pipeline.attachPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(int)*2);

    m_pipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER);

    return m_pipeline.init(m_renderGraph.getRenderPass(m_defaultPass));
}

void DefaultRenderer::cleanup()
{
    AbstractRenderer::cleanup();
    m_pipeline.destroy();
    Sprite::cleanupRendering();
}

}
