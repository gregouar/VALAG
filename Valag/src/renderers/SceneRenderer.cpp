#include "Valag/renderers/SceneRenderer.h"

#include "Valag/core/VApp.h"
#include "Valag/assets/MeshAsset.h"
#include "Valag/renderers/PBRToolbox.h"
#include "Valag/scene/IsoSpriteEntity.h"
#include "Valag/scene/MeshEntity.h"
#include "Valag/utils/Logger.h"

#include <sstream>

namespace vlg
{

const char *SceneRenderer::ISOSPRITE_DEFERRED_VERTSHADERFILE = "isoSpriteShader.vert.spv";
const char *SceneRenderer::ISOSPRITE_DEFERRED_FRAGSHADERFILE = "isoSpriteShader.frag.spv";
const char *SceneRenderer::MESH_DEFERRED_VERTSHADERFILE = "meshShader.vert.spv";
const char *SceneRenderer::MESH_DEFERRED_FRAGSHADERFILE = "meshShader.frag.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADETECT_VERTSHADERFILE = "isoSpriteAlphaDetection.vert.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADETECT_FRAGSHADERFILE = "isoSpriteAlphaDetection.frag.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADEFERRED_VERTSHADERFILE = "isoSpriteShader.vert.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADEFERRED_FRAGSHADERFILE = "isoSpriteAlphaShader.frag.spv";
const char *SceneRenderer::LIGHTING_VERTSHADERFILE = "lighting.vert.spv";
const char *SceneRenderer::LIGHTING_FRAGSHADERFILE = "lighting.frag.spv";
const char *SceneRenderer::AMBIENTLIGHTING_VERTSHADERFILE = "ambientLighting.vert.spv";
const char *SceneRenderer::AMBIENTLIGHTING_FRAGSHADERFILE = "ambientLighting.frag.spv";
const char *SceneRenderer::TONEMAPPING_VERTSHADERFILE = "toneMapping.vert.spv";
const char *SceneRenderer::TONEMAPPING_FRAGSHADERFILE = "toneMapping.frag.spv";


SceneRenderer::SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) :
    AbstractRenderer(targetWindow, name, order)
    //m_sampler(VK_NULL_HANDLE),
    //m_ambientLightingDescriptorLayout(VK_NULL_HANDLE)
{
    this->init();
}

SceneRenderer::~SceneRenderer()
{
    this->cleanup();
}

void SceneRenderer::addToSpritesVbo(const IsoSpriteDatum &datum)
{
    m_spritesVbos[m_curFrameIndex].push_back(datum);
}

void SceneRenderer::addToMeshesVbo(VMesh* mesh, const MeshDatum &datum)
{
    auto foundedVbo = m_meshesVbos[m_curFrameIndex].find(mesh);
    if(foundedVbo == m_meshesVbos[m_curFrameIndex].end())
        foundedVbo = m_meshesVbos[m_curFrameIndex].insert(foundedVbo, {mesh, DynamicVBO<MeshDatum>(4)});
    foundedVbo->second.push_back(datum);
    //m_spritesVbos[m_curFrameIndex].push_back(datum);
}

void SceneRenderer::addToLightsVbo(const LightDatum &datum)
{
    m_lightsVbos[m_curFrameIndex].push_back(datum);
}

void SceneRenderer::setAmbientLightingData(const AmbientLightingData &data)
{
    m_ambientLightingData = data;
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
    ///Probably it should be put elsewhere...
    this->updateUbos(imageIndex);

    size_t  spritesVboSize = m_spritesVbos[m_curFrameIndex].uploadVBO();
    VBuffer spritesInstancesVB = m_spritesVbos[m_curFrameIndex].getBuffer();

    std::vector<size_t>     meshesVboSize(m_meshesVbos[m_curFrameIndex].size());
    std::vector<VBuffer>    meshesInstanceVB(m_meshesVbos[m_curFrameIndex].size());

    ///I could put this in recording to save time
    size_t i = 0;
    for(auto &mesh : m_meshesVbos[m_curFrameIndex])
    {
        meshesVboSize[i]    = mesh.second.uploadVBO();
        meshesInstanceVB[i] = mesh.second.getBuffer();
        i++;

    }

    VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                        VTexturesManager::descriptorSet(m_curFrameIndex) };

    VkCommandBuffer cmb = m_renderGraph.startRecording(m_deferredPass, imageIndex, m_curFrameIndex);


        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_deferredSpritesPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);


        m_deferredMeshesPipeline.bind(cmb);

        size_t j = 0;
        for(auto &mesh : m_meshesVbos[m_curFrameIndex])
        {
            if(meshesVboSize[j] != 0)
            {
                VBuffer vertexBuffer = mesh.first->getVertexBuffer();
                VBuffer indexBuffer = mesh.first->getIndexBuffer();

                ///I could bind both in one call though
                //Mesh vertex buffer
                vkCmdBindVertexBuffers(cmb, 0, 1, &vertexBuffer.buffer,
                                                  &vertexBuffer.offset);
                //Instance vertex buffer
                vkCmdBindVertexBuffers(cmb, 1, 1, &meshesInstanceVB[j].buffer,
                                                  &meshesInstanceVB[j].offset);

                vkCmdBindIndexBuffer(cmb, indexBuffer.buffer,
                                          indexBuffer.offset, VK_INDEX_TYPE_UINT16);

                vkCmdDrawIndexed(cmb, mesh.first->getIndexCount(), meshesVboSize[j], 0, 0, 0);
            }

            j++;
        }

        if(spritesVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &spritesInstancesVB.buffer, &spritesInstancesVB.offset);

            m_deferredSpritesPipeline.bind(cmb);

            vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_deferredPass))
        return (false);

    cmb = m_renderGraph.startRecording(m_alphaDetectPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_alphaDetectPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);

        if(spritesVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &spritesInstancesVB.buffer, &spritesInstancesVB.offset);

            m_alphaDetectPipeline.bind(cmb);

            vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_alphaDetectPass))
        return (false);


    VkDescriptorSet descriptorSetsBis[] = { m_renderView.getDescriptorSet(m_curFrameIndex),
                                            VTexturesManager::descriptorSet(m_curFrameIndex),
                                            m_renderGraph.getDescriptorSet(m_alphaDeferredPass, imageIndex) };

    cmb = m_renderGraph.startRecording(m_alphaDeferredPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_alphaDeferredPipeline.getLayout(),0,3, descriptorSetsBis, 0, nullptr);

        if(spritesVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &spritesInstancesVB.buffer, &spritesInstancesVB.offset);

            m_alphaDeferredPipeline.bind(cmb);

            vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_alphaDeferredPass))
        return (false);


    size_t  lightsVboSize       = m_lightsVbos[m_curFrameIndex].uploadVBO();
    VBuffer lightsInstancesVB   = m_lightsVbos[m_curFrameIndex].getBuffer();

    VkDescriptorSet lightDescriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                             m_renderGraph.getDescriptorSet(m_lightingPass,m_curFrameIndex) };

    cmb = m_renderGraph.startRecording(m_lightingPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_lightingPipeline.getLayout(),0,2, lightDescriptorSets, 0, nullptr);

        m_lightingPipeline.bind(cmb);

        if(lightsVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &lightsInstancesVB.buffer, &lightsInstancesVB.offset);

            m_lightingPipeline.bind(cmb);

            vkCmdDraw(cmb, LIGHT_TRIANGLECOUNT+2, lightsVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_lightingPass))
        return (false);

    ///Test
    lightDescriptorSets[1] = m_renderGraph.getDescriptorSet(m_alphaLightingPass,m_curFrameIndex);

    cmb = m_renderGraph.startRecording(m_alphaLightingPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_alphaLightingPipeline.getLayout(),0,2, lightDescriptorSets, 0, nullptr);

        m_alphaLightingPipeline.bind(cmb);

        if(lightsVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &lightsInstancesVB.buffer, &lightsInstancesVB.offset);

            m_alphaLightingPipeline.bind(cmb);

            vkCmdDraw(cmb, LIGHT_TRIANGLECOUNT+2, lightsVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_alphaLightingPass))
        return (false);

    return (true);
}


bool SceneRenderer::recordAmbientLightingCmb(uint32_t imageIndex)
{
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_ambientLightingPass, imageIndex, m_curFrameIndex);

        m_ambientLightingPipeline.bind(cmb);

        VkDescriptorSet descSets[] = {m_renderGraph.getDescriptorSet(m_ambientLightingPass,imageIndex)/*,
                                      m_ambientLightingDescriptorSet*/};
        vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ambientLightingPipeline.getLayout(),
                                0, 1, descSets, 0, NULL);
        vkCmdDraw(cmb, 3, 1, 0, 0);

    return m_renderGraph.endRecording(m_ambientLightingPass);
}

bool SceneRenderer::updateUbos(uint32_t imageIndex)
{
    /*m_ambientLightingUbo[imageIndex].viewPos = glm::vec4(m_renderView.getTranslate(),0.0);

    std::cout<<m_ambientLightingUbo[imageIndex].viewPos.x<<" "
            <<m_ambientLightingUbo[imageIndex].viewPos.y<<" "
            <<m_ambientLightingUbo[imageIndex].viewPos.z<<std::endl;*/

    VBuffersAllocator::writeBuffer(m_ambientLightingUbo[imageIndex],
                                  &m_ambientLightingData,
                                   sizeof(AmbientLightingData));
    return (true);
}

bool SceneRenderer::init()
{
    m_renderView.setDepthFactor(1024*1024);
    m_renderView.setScreenOffset(glm::vec3(0.0f, 0.0f, 0.5f));

    m_spritesVbos.resize(m_targetWindow->getFramesCount(),
                         DynamicVBO<IsoSpriteDatum>(1024));
    m_meshesVbos.resize(m_targetWindow->getFramesCount());
    m_lightsVbos.resize(m_targetWindow->getFramesCount(),
                        DynamicVBO<LightDatum>(512));

    //if(!this->createSampler())
       // return (false);

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

void SceneRenderer::prepareRenderPass()
{
    this->prepareDeferredRenderPass();
    this->prepareAlphaDetectRenderPass();
    this->prepareAlphaDeferredRenderPass();
    this->prepareLightingRenderPass();
    this->prepareAlphaLightingRenderPass();
    this->prepareAmbientLightingRenderPass();
    this->prepareToneMappingRenderPass();

    /*m_renderGraph.connectRenderPasses(m_deferredPass, m_ambientLightingPass);
    m_renderGraph.connectRenderPasses(m_deferredPass, m_alphaDetectPass);
    m_renderGraph.connectRenderPasses(m_alphaDetectPass, m_alphaDeferredPass);
    m_renderGraph.connectRenderPasses(m_alphaDeferredPass, m_ambientLightingPass);
    m_renderGraph.connectRenderPasses(m_ambientLightingPass, m_toneMappingPass);*/
}

bool SceneRenderer::createGraphicsPipeline()
{
    if(!this->createDeferredSpritesPipeline())
        return (false);
    if(!this->createDeferredMeshesPipeline())
        return (false);
    if(!this->createAlphaDetectPipeline())
        return (false);
    if(!this->createAlphaDeferredPipeline())
        return (false);
    if(!this->createLightingPipeline())
        return (false);
    if(!this->createAlphaLightingPipeline())
        return (false);
    if(!this->createAmbientLightingPipeline())
        return (false);
    if(!this->createToneMappingPipeline())
        return (false);

    return (true);
}

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
                if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_D24_UNORM_S8_UINT,
                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, m_deferredDepthAttachments[i]))
                    return (false);

                if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8_UNORM,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_alphaDetectAttachments[i]))
                    return (false);
            }

            if(!
                VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_albedoAttachments[a][i]) &
                VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_positionAttachments[a][i]) &
                VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SFLOAT,
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
}

void SceneRenderer::prepareDeferredRenderPass()
{
    m_deferredPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderGraph.addNewAttachments(m_deferredPass, m_albedoAttachments[0]);
    m_renderGraph.addNewAttachments(m_deferredPass, m_positionAttachments[0]);
    m_renderGraph.addNewAttachments(m_deferredPass, m_normalAttachments[0]);
    m_renderGraph.addNewAttachments(m_deferredPass, m_rmtAttachments[0]);
    m_renderGraph.addNewAttachments(m_deferredPass, m_deferredDepthAttachments);
}

void SceneRenderer::prepareAlphaDetectRenderPass()
{
    m_alphaDetectPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderGraph.addNewAttachments(m_alphaDetectPass, m_alphaDetectAttachments, VK_ATTACHMENT_STORE_OP_STORE);
    m_renderGraph.transferAttachmentsToAttachments(m_deferredPass, m_alphaDetectPass, 4);
}

void SceneRenderer::prepareAlphaDeferredRenderPass()
{
    m_alphaDeferredPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderGraph.addNewAttachments(m_alphaDeferredPass, m_albedoAttachments[1]);
    m_renderGraph.addNewAttachments(m_alphaDeferredPass, m_positionAttachments[1]);
    m_renderGraph.addNewAttachments(m_alphaDeferredPass, m_normalAttachments[1]);
    m_renderGraph.addNewAttachments(m_alphaDeferredPass, m_rmtAttachments[1]);
    m_renderGraph.transferAttachmentsToAttachments(m_alphaDetectPass, m_alphaDeferredPass, 1);

    m_renderGraph.transferAttachmentsToUniforms(m_alphaDetectPass, m_alphaDeferredPass, 0);
}

void SceneRenderer::prepareLightingRenderPass()
{
    m_lightingPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderGraph.addNewAttachments(m_lightingPass, m_hdrAttachements[0]);
    //m_renderGraph.addNewAttachments(m_lightingPass, m_hdrAttachements[1]);

    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_lightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_lightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_lightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_lightingPass, 3);

    /*m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_lightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_lightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_lightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_lightingPass, 3);*/


    ///This should be elsewhere
    size_t imagesCount = m_targetWindow->getSwapchainSize();
    m_ambientLightingUbo.resize(imagesCount);
    for(auto &buffer : m_ambientLightingUbo)
    {
        VBuffersAllocator::allocBuffer(sizeof(AmbientLightingData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                       buffer);
    }
    m_renderGraph.addNewUniforms(m_lightingPass, m_ambientLightingUbo);
}

void SceneRenderer::prepareAlphaLightingRenderPass()
{
    m_alphaLightingPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderGraph.addNewAttachments(m_alphaLightingPass, m_hdrAttachements[1]);
    m_renderGraph.transferAttachmentsToAttachments(m_alphaDeferredPass, m_alphaLightingPass, 4);

    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_alphaLightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_alphaLightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_alphaLightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_alphaLightingPass, 3);

    m_renderGraph.addNewUniforms(m_alphaLightingPass, m_ambientLightingUbo);

}

void SceneRenderer::prepareAmbientLightingRenderPass()
{
    m_ambientLightingPass = m_renderGraph.addRenderPass(0);

    //m_renderGraph.addNewAttachments(m_ambientLightingPass, m_hdrAttachements[0]);
    //m_renderGraph.addNewAttachments(m_ambientLightingPass, m_hdrAttachements[1]);

    m_renderGraph.transferAttachmentsToAttachments(m_lightingPass, m_ambientLightingPass, 0);
    //m_renderGraph.transferAttachmentsToAttachments(m_lightingPass, m_ambientLightingPass, 1);
    m_renderGraph.transferAttachmentsToAttachments(m_alphaLightingPass, m_ambientLightingPass, 0);

    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 3);

    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_ambientLightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_ambientLightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_ambientLightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_ambientLightingPass, 3);

    m_renderGraph.addNewUniforms(m_ambientLightingPass, m_ambientLightingUbo);

    size_t imagesCount = m_targetWindow->getSwapchainSize();
    std::vector<VkImageView> brdflut(imagesCount, PBRToolbox::getBrdflut().view);
    m_renderGraph.addNewUniforms(m_ambientLightingPass, brdflut);
}

void SceneRenderer::prepareToneMappingRenderPass()
{
    m_toneMappingPass = m_renderGraph.addRenderPass(0);

    m_renderGraph.addNewAttachments(m_toneMappingPass, m_targetWindow->getSwapchainAttachments(), VK_ATTACHMENT_STORE_OP_STORE);
    m_renderGraph.addNewAttachments(m_toneMappingPass, m_targetWindow->getSwapchainDepthAttachments(), VK_ATTACHMENT_STORE_OP_STORE);
    m_renderGraph.transferAttachmentsToUniforms(m_ambientLightingPass, m_toneMappingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_ambientLightingPass, m_toneMappingPass, 1);
}

bool SceneRenderer::createDeferredSpritesPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_DEFERRED_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_DEFERRED_FRAGSHADERFILE;

    m_deferredSpritesPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_deferredSpritesPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = IsoSpriteDatum::getBindingDescription();
    auto attributeDescriptions = IsoSpriteDatum::getAttributeDescriptions();
    m_deferredSpritesPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_deferredSpritesPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);

    m_deferredSpritesPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_deferredSpritesPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_deferredSpritesPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_deferredSpritesPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER);

    return m_deferredSpritesPipeline.init(  m_renderGraph.getVkRenderPass(m_deferredPass), 0,
                                            m_renderGraph.getColorAttachmentsCount(m_deferredPass));
}

bool SceneRenderer::createDeferredMeshesPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << MESH_DEFERRED_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << MESH_DEFERRED_FRAGSHADERFILE;

    m_deferredMeshesPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_deferredMeshesPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkVertexInputBindingDescription> bindingDescriptions =
                {  MeshVertex::getBindingDescription(),
                    MeshDatum::getBindingDescription() };

    auto vertexAttributeDescriptions = MeshVertex::getAttributeDescriptions();
    auto instanceAttributeDescriptions = MeshDatum::getAttributeDescriptions();

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    attributeDescriptions.insert(attributeDescriptions.end(),
                                 vertexAttributeDescriptions.begin(),
                                 vertexAttributeDescriptions.end());

    attributeDescriptions.insert(attributeDescriptions.end(),
                                 instanceAttributeDescriptions.begin(),
                                 instanceAttributeDescriptions.end());

    m_deferredMeshesPipeline.setVertexInput(bindingDescriptions.size(), bindingDescriptions.data(),
                                            attributeDescriptions.size(), attributeDescriptions.data());

    m_deferredMeshesPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);

    m_deferredMeshesPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_deferredMeshesPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_deferredMeshesPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_deferredMeshesPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER);

    m_deferredMeshesPipeline.setCullMode(VK_CULL_MODE_BACK_BIT);

    return m_deferredMeshesPipeline.init(   m_renderGraph.getVkRenderPass(m_deferredPass), 0,
                                            m_renderGraph.getColorAttachmentsCount(m_deferredPass));
}

bool SceneRenderer::createAlphaDetectPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_ALPHADETECT_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_ALPHADETECT_FRAGSHADERFILE;

    m_alphaDetectPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_alphaDetectPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = IsoSpriteDatum::getBindingDescription();
    auto attributeDescriptions = IsoSpriteDatum::getAttributeDescriptions();
    m_alphaDetectPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_alphaDetectPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);

    m_alphaDetectPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_alphaDetectPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_alphaDetectPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_alphaDetectPipeline.setDepthTest(false, true, VK_COMPARE_OP_GREATER);

    VkStencilOpState stencil = {};
    stencil.compareOp   = VK_COMPARE_OP_ALWAYS;
	stencil.failOp      = VK_STENCIL_OP_REPLACE;
	stencil.depthFailOp = VK_STENCIL_OP_REPLACE;
	stencil.passOp      = VK_STENCIL_OP_REPLACE;
	stencil.compareMask = 0xff;
	stencil.writeMask   = 0xff;
    stencil.reference   = 1;
    m_alphaDetectPipeline.setStencilTest(true, stencil);

    return m_alphaDetectPipeline.init(  m_renderGraph.getVkRenderPass(m_alphaDetectPass), 0,
                                        m_renderGraph.getColorAttachmentsCount(m_alphaDetectPass));
}

bool SceneRenderer::createAlphaDeferredPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_ALPHADEFERRED_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_ALPHADEFERRED_FRAGSHADERFILE;

    m_alphaDeferredPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_alphaDeferredPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = IsoSpriteDatum::getBindingDescription();
    auto attributeDescriptions = IsoSpriteDatum::getAttributeDescriptions();
    m_alphaDeferredPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_alphaDeferredPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);

    m_alphaDeferredPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_alphaDeferredPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_alphaDeferredPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());
    m_alphaDeferredPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_alphaDeferredPass));

    m_alphaDeferredPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER_OR_EQUAL);

    VkStencilOpState stencil = {};
    stencil.compareOp   = VK_COMPARE_OP_EQUAL;//VK_COMPARE_OP_EQUAL;
	stencil.failOp      = VK_STENCIL_OP_KEEP;
	stencil.depthFailOp = VK_STENCIL_OP_KEEP;
	stencil.passOp      = VK_STENCIL_OP_REPLACE;
	stencil.compareMask = 0xff;
	stencil.writeMask   = 0xff;
    stencil.reference   = 1;
    m_alphaDeferredPipeline.setStencilTest(true, stencil);

    return m_alphaDeferredPipeline.init(m_renderGraph.getVkRenderPass(m_alphaDeferredPass), 0,
                                          m_renderGraph.getColorAttachmentsCount(m_alphaDeferredPass));
}

bool SceneRenderer::createLightingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << LIGHTING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << LIGHTING_FRAGSHADERFILE;

    m_lightingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_lightingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = LightDatum::getBindingDescription();
    auto attributeDescriptions = LightDatum::getAttributeDescriptions();
    m_lightingPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_lightingPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, false);

    m_lightingPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_lightingPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_lightingPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_lightingPass));
    //m_deferredSpritesPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_lightingPipeline.setBlendMode(BlendMode_Add,0);
    //m_lightingPipeline.setBlendMode(BlendMode_Add,1);

    return m_lightingPipeline.init( m_renderGraph.getVkRenderPass(m_lightingPass), 0,
                                    m_renderGraph.getColorAttachmentsCount(m_lightingPass));
}

bool SceneRenderer::createAlphaLightingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << LIGHTING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << LIGHTING_FRAGSHADERFILE;

    m_alphaLightingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_alphaLightingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = LightDatum::getBindingDescription();
    auto attributeDescriptions = LightDatum::getAttributeDescriptions();
    m_alphaLightingPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_alphaLightingPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, false);

    m_alphaLightingPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_alphaLightingPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_alphaLightingPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_alphaLightingPass));
    //m_deferredSpritesPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_alphaLightingPipeline.setBlendMode(BlendMode_Add,0);

    VkStencilOpState stencil = {};
    stencil.compareOp   = VK_COMPARE_OP_EQUAL;//VK_COMPARE_OP_EQUAL;
	stencil.failOp      = VK_STENCIL_OP_KEEP;
	stencil.depthFailOp = VK_STENCIL_OP_KEEP;
	stencil.passOp      = VK_STENCIL_OP_REPLACE;
	stencil.compareMask = 0xff;
	stencil.writeMask   = 0xff;
    stencil.reference   = 1;
    m_alphaLightingPipeline.setStencilTest(true, stencil);

    return m_alphaLightingPipeline.init( m_renderGraph.getVkRenderPass(m_alphaLightingPass), 0,
                                         m_renderGraph.getColorAttachmentsCount(m_alphaLightingPass));
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
    //m_ambientLightingPipeline.attachDescriptorSetLayout(m_ambientLightingDescriptorLayout);

    m_ambientLightingPipeline.setBlendMode(BlendMode_Add,0);
    m_ambientLightingPipeline.setBlendMode(BlendMode_Add,1);

    return m_ambientLightingPipeline.init(m_renderGraph.getVkRenderPass(m_ambientLightingPass), 0,
                                          m_renderGraph.getColorAttachmentsCount(m_ambientLightingPass));
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

    return m_toneMappingPipeline.init(  m_renderGraph.getVkRenderPass(m_toneMappingPass), 0,
                                        m_renderGraph.getColorAttachmentsCount(m_toneMappingPass));
}

void SceneRenderer::cleanup()
{
    m_spritesVbos.clear();
    m_meshesVbos.clear();

    /*if(m_sampler != VK_NULL_HANDLE)
        vkDestroySampler(device, m_attachmentsSampler, nullptr);
    m_sampler = VK_NULL_HANDLE;*/

/*    if(m_ambientLightingDescriptorLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(VInstance::device(), m_ambientLightingDescriptorLayout, nullptr);
    m_ambientLightingDescriptorLayout = VK_NULL_HANDLE;*/

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

    m_deferredSpritesPipeline.destroy();
    m_deferredMeshesPipeline.destroy();
    m_alphaDetectPipeline.destroy();
    m_alphaDeferredPipeline.destroy();
    m_lightingPipeline.destroy();
    m_ambientLightingPipeline.destroy();
    m_toneMappingPipeline.destroy();

    AbstractRenderer::cleanup();
}


}
