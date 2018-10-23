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

const char *SceneRenderer::ISOSPRITE_SHADOWGEN_VERTSHADERFILE = "deferred/isoSpriteShadowGen.vert.spv";
const char *SceneRenderer::ISOSPRITE_SHADOWGEN_FRAGSHADERFILE = "deferred/isoSpriteShadowGen.frag.spv";
const char *SceneRenderer::ISOSPRITE_SHADOW_VERTSHADERFILE = "deferred/isoSpriteShadow.vert.spv";
const char *SceneRenderer::ISOSPRITE_SHADOW_FRAGSHADERFILE = "deferred/isoSpriteShadow.frag.spv";
const char *SceneRenderer::ISOSPRITE_DEFERRED_VERTSHADERFILE = "deferred/isoSpriteShader.vert.spv";
const char *SceneRenderer::ISOSPRITE_DEFERRED_FRAGSHADERFILE = "deferred/isoSpriteShader.frag.spv";
const char *SceneRenderer::MESH_DEFERRED_VERTSHADERFILE = "deferred/meshShader.vert.spv";
const char *SceneRenderer::MESH_DEFERRED_FRAGSHADERFILE = "deferred/meshShader.frag.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADETECT_VERTSHADERFILE = "deferred/isoSpriteAlphaDetection.vert.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADETECT_FRAGSHADERFILE = "deferred/isoSpriteAlphaDetection.frag.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADEFERRED_VERTSHADERFILE = "deferred/isoSpriteShader.vert.spv";
const char *SceneRenderer::ISOSPRITE_ALPHADEFERRED_FRAGSHADERFILE = "deferred/isoSpriteAlphaShader.frag.spv";
const char *SceneRenderer::SSGI_BN_VERTSHADERFILE = "lighting/ssgiBN.vert.spv";
const char *SceneRenderer::SSGI_BN_FRAGSHADERFILE = "lighting/ssgiBN.frag.spv";
const char *SceneRenderer::LIGHTING_VERTSHADERFILE = "lighting/lighting.vert.spv";
const char *SceneRenderer::LIGHTING_FRAGSHADERFILE = "lighting/lighting.frag.spv";
const char *SceneRenderer::SSGI_LIGHTING_VERTSHADERFILE = "lighting/ssgiLighting.vert.spv";
const char *SceneRenderer::SSGI_LIGHTING_FRAGSHADERFILE = "lighting/ssgiLighting.frag.spv";
const char *SceneRenderer::AMBIENTLIGHTING_VERTSHADERFILE = "lighting/ambientLighting.vert.spv";
const char *SceneRenderer::AMBIENTLIGHTING_FRAGSHADERFILE = "lighting/ambientLighting.frag.spv";
const char *SceneRenderer::TONEMAPPING_VERTSHADERFILE = "toneMapping.vert.spv";
const char *SceneRenderer::TONEMAPPING_FRAGSHADERFILE = "toneMapping.frag.spv";
const char *SceneRenderer::BLUR_VERTSHADERFILE = "smartBlur.vert.spv";
const char *SceneRenderer::BLUR_FRAGSHADERFILE = "smartBlur.frag.spv";


SceneRenderer::SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) :
    AbstractRenderer(targetWindow, name, order)
    //m_sampler(VK_NULL_HANDLE),
    //m_ambientLightingDescriptorLayout(VK_NULL_HANDLE)
{
    //m_useDynamicView = true;
    this->init();
}

SceneRenderer::~SceneRenderer()
{
    this->cleanup();
}

void SceneRenderer::addRenderingInstance(SceneRenderingInstance *renderingInstance)
{
    m_renderingInstances.push_back(renderingInstance);
}

void SceneRenderer::addShadowMapToRender(VRenderTarget* shadowMap, const LightDatum &datum)
{
    m_shadowMapsToRender.push_back({shadowMap, datum});
    m_renderGraph.addDynamicRenderTarget(m_shadowMapsPass,shadowMap);
}

void SceneRenderer::addSpriteShadowToRender(VRenderTarget* spriteShadow, const SpriteShadowGenerationDatum &datum)
{
    m_renderGraph.addDynamicRenderTarget(m_spriteShadowsPass,spriteShadow);
    m_spriteShadowGenerationVbos[m_curFrameIndex]->push_back(datum);

    //m_spriteShadowsToRender.push_back({spriteShadow, datum});
}

void SceneRenderer::addToSpriteShadowsVbo(const IsoSpriteShadowDatum &datum)
{
    m_spriteShadowsVbos[m_curFrameIndex]->push_back(datum);
}

void SceneRenderer::addToMeshShadowsVbo(VMesh *mesh, const MeshDatum &datum)
{

}

void SceneRenderer::addToSpritesVbo(const IsoSpriteDatum &datum)
{
    m_spritesVbos[m_curFrameIndex]->push_back(datum);
}

void SceneRenderer::addToMeshesVbo(VMesh* mesh, const MeshDatum &datum)
{
    auto foundedVbo = m_meshesVbos[m_curFrameIndex].find(mesh);
    if(foundedVbo == m_meshesVbos[m_curFrameIndex].end())
        foundedVbo = m_meshesVbos[m_curFrameIndex].insert(foundedVbo, {mesh, new DynamicVBO<MeshDatum>(4)});
    foundedVbo->second->push_back(datum);
}

void SceneRenderer::addToLightsVbo(const LightDatum &datum)
{
    m_lightsVbos[m_curFrameIndex]->push_back(datum);
}

/*void SceneRenderer::setAmbientLightingData(const AmbientLightingData &data)
{
    m_ambientLightingData = data;
}*/

size_t SceneRenderer::getSpritesVboSize()
{
    return m_spritesVbos[m_curFrameIndex]->getSize();
}

size_t SceneRenderer::getMeshesVboSize(VMesh *mesh)
{
    auto foundedVbo = m_meshesVbos[m_curFrameIndex].find(mesh);
    if(foundedVbo == m_meshesVbos[m_curFrameIndex].end())
        return (0);
    return foundedVbo->second->getSize();
}

size_t SceneRenderer::getLightsVboSize()
{
    return m_lightsVbos[m_curFrameIndex]->getSize();
}

VRenderPass *SceneRenderer::getSpriteShadowsRenderPass()
{
    return m_renderGraph.getRenderPass(m_spriteShadowsPass);
}

VRenderPass *SceneRenderer::getShadowMapsRenderPass()
{
    return m_renderGraph.getRenderPass(m_shadowMapsPass);
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
    this->recordShadowCmb(imageIndex);

    bool r = true;

    if(!this->recordDeferredCmb(imageIndex))
        r = false;
    if(!this->recordLightingCmb(imageIndex))
        r = false;
    if(!this->recordAmbientLightingCmb(imageIndex))
        r = false;

    for(auto renderingInstance : m_renderingInstances)
        delete renderingInstance;
    m_renderingInstances.clear();

    return r;
}

bool SceneRenderer::recordShadowCmb(uint32_t imageIndex)
{
    for(auto renderingInstance : m_renderingInstances)
        renderingInstance->recordShadows(this, imageIndex); //Probably a bad name for that

    size_t  spriteShadowGenVboSize = m_spriteShadowGenerationVbos[m_curFrameIndex]->uploadVBO();
    VBuffer spriteShadowGenInstancesVB = m_spriteShadowGenerationVbos[m_curFrameIndex]->getBuffer();

    VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                        VTexturesManager::descriptorSet(m_curFrameIndex) };

    //Compute shadow maps here
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_spriteShadowsPass, 0, m_curFrameIndex);
        if(spriteShadowGenVboSize > 0)
        {
            m_spriteShadowsGenPipeline.bind(cmb);

            vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_spriteShadowsGenPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);

            vkCmdBindVertexBuffers(cmb, 0, 1,   &spriteShadowGenInstancesVB.buffer,
                                                &spriteShadowGenInstancesVB.offset);

            size_t i = 0;
            while(m_renderGraph.nextRenderTarget(m_spriteShadowsPass))
            {
                m_spriteShadowsGenPipeline.updateViewport(cmb, {0,0},
                        m_renderGraph.getExtent(m_spriteShadowsPass));
                vkCmdDraw(cmb, 3, 1, 0, i);
                i++;
            }
        }

    m_renderGraph.endRecording(m_spriteShadowsPass);
    //m_spriteShadowsToRender.clear();


    size_t  spritesVboSize      = m_spriteShadowsVbos[m_curFrameIndex]->uploadVBO();
    VBuffer spritesInstancesVB  = m_spriteShadowsVbos[m_curFrameIndex]->getBuffer();

    //Start recording
    cmb = m_renderGraph.startRecording(m_shadowMapsPass, /*imageIndex*/ 0, m_curFrameIndex);

    //for(auto renderingInstance : m_renderingInstances)
    {
        //Setup viewport (?)
        //m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);

        //Render shadow map

        size_t i = 0;
        while(m_renderGraph.nextRenderTarget(m_shadowMapsPass))
        {
            if(spritesVboSize != 0)
            {
                vkCmdBindVertexBuffers(cmb, 0, 1, &spritesInstancesVB.buffer, &spritesInstancesVB.offset);
                m_spriteShadowsPipeline.bind(cmb);

                vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        m_spriteShadowsPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);

                for(auto renderingInstance : m_renderingInstances)
                {
                    ///m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);
                    ///I'll need to find something smart to do
                    m_spriteShadowsPipeline.updateViewport(cmb, {0,0},
                            m_renderGraph.getExtent(m_shadowMapsPass));

                    renderingInstance->pushCamPosAndZoom(cmb, m_spriteShadowsPipeline.getLayout(),
                                                        VK_SHADER_STAGE_VERTEX_BIT);

                    vkCmdDraw(cmb, 4, spritesVboSize/*renderingInstance->getSpritesVboSize()*/,
                                   0, 0/*renderingInstance->getSpritesVboOffset()*/);
                }
            }

            i++;
        }
    }
    m_renderGraph.endRecording(m_shadowMapsPass);
    m_shadowMapsToRender.clear();

    return (true);
}

bool SceneRenderer::recordDeferredCmb(uint32_t imageIndex)
{
    size_t  spritesVboSize      = m_spritesVbos[m_curFrameIndex]->uploadVBO();
    VBuffer spritesInstancesVB  = m_spritesVbos[m_curFrameIndex]->getBuffer();

    std::vector<size_t>     meshesVboSize(m_meshesVbos[m_curFrameIndex].size());
    std::vector<VBuffer>    meshesInstanceVB(m_meshesVbos[m_curFrameIndex].size());

    ///I could put this in recording to save time
    size_t i = 0;
    for(auto &mesh : m_meshesVbos[m_curFrameIndex])
    {
        meshesVboSize[i]    = mesh.second->uploadVBO();
        meshesInstanceVB[i] = mesh.second->getBuffer();
        i++;

    }

    VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                        VTexturesManager::descriptorSet(m_curFrameIndex) };

    /// Opac sprites & meshes
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

                for(auto renderingInstance : m_renderingInstances)
                {
                    m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);
                    renderingInstance->pushCamPosAndZoom(cmb, m_deferredMeshesPipeline.getLayout());

                    vkCmdDrawIndexed(cmb, mesh.first->getIndexCount(), renderingInstance->getMeshesVboSize(mesh.first),
                                     0, 0, renderingInstance->getMeshesVboOffset(mesh.first));
                }
                //vkCmdDrawIndexed(cmb, mesh.first->getIndexCount(), meshesVboSize[j], 0, 0, 0);
            }

            j++;
        }

        if(spritesVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &spritesInstancesVB.buffer, &spritesInstancesVB.offset);

            m_deferredSpritesPipeline.bind(cmb);

            for(auto renderingInstance : m_renderingInstances)
            {
                m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);
                renderingInstance->pushCamPosAndZoom(cmb, m_deferredSpritesPipeline.getLayout(),
                                                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

                vkCmdDraw(cmb, 4, renderingInstance->getSpritesVboSize(),
                               0, renderingInstance->getSpritesVboOffset());
            }
            //vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_deferredPass))
        return (false);


    /// Detection of alpha fragments
    cmb = m_renderGraph.startRecording(m_alphaDetectPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_alphaDetectPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);

        if(spritesVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &spritesInstancesVB.buffer, &spritesInstancesVB.offset);

            m_alphaDetectPipeline.bind(cmb);

            for(auto renderingInstance : m_renderingInstances)
            {
                m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);
                renderingInstance->pushCamPosAndZoom(cmb, m_alphaDetectPipeline.getLayout());

                vkCmdDraw(cmb, 4, renderingInstance->getSpritesVboSize(),
                               0, renderingInstance->getSpritesVboOffset());
            }

            //vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_alphaDetectPass))
        return (false);

    VkDescriptorSet descriptorSetsBis[] = { m_renderView.getDescriptorSet(m_curFrameIndex),
                                            VTexturesManager::descriptorSet(m_curFrameIndex),
                                            m_renderGraph.getDescriptorSet(m_alphaDeferredPass, imageIndex) };

    /// Alpha sprites
    cmb = m_renderGraph.startRecording(m_alphaDeferredPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_alphaDeferredPipeline.getLayout(),0,3, descriptorSetsBis, 0, nullptr);

        if(spritesVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &spritesInstancesVB.buffer, &spritesInstancesVB.offset);

            m_alphaDeferredPipeline.bind(cmb);

            for(auto renderingInstance : m_renderingInstances)
            {
                m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);
                renderingInstance->pushCamPosAndZoom(cmb, m_alphaDeferredPipeline.getLayout(),
                                                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

                vkCmdDraw(cmb, 4, renderingInstance->getSpritesVboSize(),
                               0, renderingInstance->getSpritesVboOffset());
            }
            //vkCmdDraw(cmb, 4, spritesVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_alphaDeferredPass))
        return (false);

    return (true);
}

bool SceneRenderer::recordLightingCmb(uint32_t imageIndex)
{
    size_t  lightsVboSize       = m_lightsVbos[m_curFrameIndex]->uploadVBO();
    VBuffer lightsInstancesVB   = m_lightsVbos[m_curFrameIndex]->getBuffer();

    VkDescriptorSet lightDescriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                             VTexturesManager::descriptorSet(m_curFrameIndex),
                                             m_renderGraph.getDescriptorSet(m_lightingPass,imageIndex/*m_curFrameIndex*/) };

    /// Lighting of opac fragments
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_lightingPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_lightingPipeline.getLayout(),0,3, lightDescriptorSets, 0, nullptr);

        m_lightingPipeline.bind(cmb);

        if(lightsVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &lightsInstancesVB.buffer, &lightsInstancesVB.offset);

            m_lightingPipeline.bind(cmb);

            for(auto renderingInstance : m_renderingInstances)
            {
                m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);
                renderingInstance->pushCamPosAndZoom(cmb, m_lightingPipeline.getLayout(),
                                                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

                vkCmdDraw(cmb, LIGHT_TRIANGLECOUNT+2, renderingInstance->getLightsVboSize(),
                               0, renderingInstance->getLightsVboOffset());
            }
            //vkCmdDraw(cmb, LIGHT_TRIANGLECOUNT+2, lightsVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_lightingPass))
        return (false);


    /// Lighting of alpha fragments
    lightDescriptorSets[2] = m_renderGraph.getDescriptorSet(m_alphaLightingPass,imageIndex/**m_curFrameIndex**/);

    cmb = m_renderGraph.startRecording(m_alphaLightingPass, imageIndex, m_curFrameIndex);

        vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_alphaLightingPipeline.getLayout(),0,3, lightDescriptorSets, 0, nullptr);

        m_alphaLightingPipeline.bind(cmb);

        if(lightsVboSize != 0)
        {
            vkCmdBindVertexBuffers(cmb, 0, 1, &lightsInstancesVB.buffer, &lightsInstancesVB.offset);

            m_alphaLightingPipeline.bind(cmb);

            for(auto renderingInstance : m_renderingInstances)
            {
                m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);
                renderingInstance->pushCamPosAndZoom(cmb, m_alphaLightingPipeline.getLayout(),
                                                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

                vkCmdDraw(cmb, LIGHT_TRIANGLECOUNT+2, renderingInstance->getLightsVboSize(),
                               0, renderingInstance->getLightsVboOffset());
            }
            //vkCmdDraw(cmb, LIGHT_TRIANGLECOUNT+2, lightsVboSize, 0, 0);
        }

    if(!m_renderGraph.endRecording(m_alphaLightingPass))
        return (false);

    return (true);
}

bool SceneRenderer::recordSsgiCmb(uint32_t imageIndex)
{
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_ssgiBNPass, imageIndex, m_curFrameIndex);

        m_ssgiBNPipeline.bind(cmb);

        ///m_renderView set shouldnt be by IMAGE !!! Need to update renderView or split set in two !!
        VkDescriptorSet descSets[] = {  m_renderView.getDescriptorSet(m_curFrameIndex),
                                        m_renderGraph.getDescriptorSet(m_ssgiBNPass,imageIndex)};

        vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ssgiBNPipeline.getLayout(),
                                0, 2, descSets, 0, NULL);

        int pc = static_cast<int>(imageIndex);
        vkCmdPushConstants(cmb, m_ssgiBNPipeline.getLayout(),
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int), (void*)&pc);

        vkCmdDraw(cmb, 3, 1, 0, 0);

    if(!m_renderGraph.endRecording(m_ssgiBNPass))
        return (false);


    ///Blur
    for(size_t i = 0 ; i < 2 ; ++i)
    {
        cmb = m_renderGraph.startRecording(m_ssgiBNBlurPasses[i], imageIndex, m_curFrameIndex);

            m_ssgiBNBlurPipelines[i].bind(cmb);

            VkDescriptorSet descSets[] = {m_renderGraph.getDescriptorSet(m_ssgiBNBlurPasses[i],imageIndex)};

            vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ssgiBNBlurPipelines[i].getLayout(),
                                    0, 1, descSets, 0, NULL);

            vkCmdDraw(cmb, 3, 1, 0, 0);

        if(!m_renderGraph.endRecording(m_ssgiBNBlurPasses[i]))
            return (false);
    }

    return (true);
}

bool SceneRenderer::recordAmbientLightingCmb(uint32_t imageIndex)
{
    VkCommandBuffer cmb = m_renderGraph.startRecording(m_ambientLightingPass, imageIndex, m_curFrameIndex);

        m_ambientLightingPipeline.bind(cmb);

        VkDescriptorSet descSets[] = {m_renderGraph.getDescriptorSet(m_ambientLightingPass,imageIndex)};

        vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ambientLightingPipeline.getLayout(),
                                0, 1, descSets, 0, NULL);

        for(auto renderingInstance : m_renderingInstances)
        {
            m_renderView.setupViewport(renderingInstance->getViewInfo(), cmb);
            renderingInstance->pushCamPosAndZoom(cmb, m_ambientLightingPipeline.getLayout(),
                                                VK_SHADER_STAGE_FRAGMENT_BIT);

            VkDescriptorSet descSetsBis[] = {renderingInstance->getRenderingData()->getAmbientLightingDescSet(m_curFrameIndex)};

            vkCmdBindDescriptorSets(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ambientLightingPipeline.getLayout(),
                                    1, 1, descSetsBis, 0, NULL);

            vkCmdDraw(cmb, 3, 1, 0, 0);
        }

    return m_renderGraph.endRecording(m_ambientLightingPass);
}

/*bool SceneRenderer::updateUbos(uint32_t imageIndex)
{
    VBuffersAllocator::writeBuffer(m_ambientLightingUbo[imageIndex],
                                  &m_ambientLightingData,
                                   sizeof(AmbientLightingData));
    return (true);
}*/

bool SceneRenderer::init()
{
    m_renderView.setDepthFactor(1024*1024);
    m_renderView.setScreenOffset(glm::vec3(0.0f, 0.0f, 0.5f));

    m_spriteShadowGenerationVbos.resize(m_targetWindow->getFramesCount());
    for(auto &vbo : m_spriteShadowGenerationVbos)
        vbo = new DynamicVBO<SpriteShadowGenerationDatum>(32);

    m_spriteShadowsVbos.resize(m_targetWindow->getFramesCount());
    for(auto &vbo : m_spriteShadowsVbos)
        vbo = new DynamicVBO<IsoSpriteShadowDatum>(128);

    m_spritesVbos.resize(m_targetWindow->getFramesCount());
    for(auto &vbo : m_spritesVbos)
        vbo = new DynamicVBO<IsoSpriteDatum>(256);

    m_meshesVbos.resize(m_targetWindow->getFramesCount());

    m_lightsVbos.resize(m_targetWindow->getFramesCount());
    for(auto &vbo : m_lightsVbos)
        vbo = new DynamicVBO<LightDatum>(128);

    //if(!this->createSampler())
       // return (false);

    if(!this->createAttachments())
        return (false);

    if(!AbstractRenderer::init())
        return (false);

   // m_ambientLightingDescVersion.resize(m_targetWindow->getSwapchainSize());
    for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        this->recordSsgiCmb(i);
        //this->recordAmbientLightingCmb(i);
        this->recordToneMappingCmb(i);
    }

    return (true);
}

void SceneRenderer::prepareRenderPass()
{
    this->prepareShadowRenderPass();
    this->prepareDeferredRenderPass();
    this->prepareAlphaDetectRenderPass();
    this->prepareAlphaDeferredRenderPass();
    this->prepareSsgiBNRenderPasses();
    this->prepareLightingRenderPass();
    this->prepareAlphaLightingRenderPass();
    this->prepareSsgiLightingRenderPass();
    this->prepareAmbientLightingRenderPass();
    this->prepareToneMappingRenderPass();
}

bool SceneRenderer::createGraphicsPipeline()
{
    if(!this->createSpriteShadowsGenPipeline())
        return (false);
    if(!this->createSpriteShadowsPipeline())
        return (false);
    if(!this->createDeferredSpritesPipeline())
        return (false);
    if(!this->createDeferredMeshesPipeline())
        return (false);
    if(!this->createAlphaDetectPipeline())
        return (false);
    if(!this->createAlphaDeferredPipeline())
        return (false);
    if(!this->createSsgiBNPipelines())
        return (false);
    if(!this->createLightingPipeline())
        return (false);
    if(!this->createAlphaLightingPipeline())
        return (false);
    if(!this->createSsgiLightingPipeline())
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
    //m_alphaDetectAttachments.resize(imagesCount);

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

                /*if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8_UNORM,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_alphaDetectAttachments[i]))
                    return (false);*/
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

    //I should probably change this since it's not accu anymore (and use multibuffering)
    if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SNORM,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_ssgiAccuBentNormalsAttachment))
        return (false);

    //We put the attachment in read only for the first pass
    /*VulkanHelpers::transitionImageLayout(m_ssgiAccuBentNormalsAttachment.image.vkImage, 0, VK_FORMAT_R16G16B16A16_SNORM,
                                         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);*/

    if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SFLOAT,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_ssgiAccuLightingAttachment))
        return (false);

    for(size_t i = 0 ; i < NBR_SSGI_SAMPLES ; ++i)
        if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_UNORM,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_ssgiCollisionsAttachments[i]))
            return (false);

    for(size_t i = 0 ; i < 2 ; ++i)
        if(!VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SNORM,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_SSGIBlurBentNormalsAttachments[i]))
            return (false);

    return (true);
}

void SceneRenderer::prepareShadowRenderPass()
{
    VFramebufferAttachmentType spriteShadowType;
    spriteShadowType.format = VK_FORMAT_R8G8B8A8_UNORM;
    spriteShadowType.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_spriteShadowsPass = m_renderGraph.addDynamicRenderPass();
    m_renderGraph.addAttachmentType(m_spriteShadowsPass, spriteShadowType,
                                    VK_ATTACHMENT_STORE_OP_STORE, false);


    VFramebufferAttachmentType shadowMapType;
    shadowMapType.format = VK_FORMAT_D24_UNORM_S8_UINT;
    shadowMapType.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    m_shadowMapsPass = m_renderGraph.addDynamicRenderPass();
    m_renderGraph.addAttachmentType(m_shadowMapsPass, shadowMapType,
                                    VK_ATTACHMENT_STORE_OP_STORE, false);
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

    m_renderGraph.transferAttachmentsToAttachments(m_deferredPass, m_alphaDetectPass, 1); //We will store existence of truly transparent pixel in position.a
    //m_renderGraph.addNewAttachments(m_alphaDetectPass, m_alphaDetectAttachments, VK_ATTACHMENT_STORE_OP_STORE);//
    //m_renderGraph.transferAttachmentsToAttachments(m_deferredPass, m_alphaDetectPass, 4);
}

void SceneRenderer::prepareAlphaDeferredRenderPass()
{
    m_alphaDeferredPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderGraph.addNewAttachments(m_alphaDeferredPass, m_albedoAttachments[1]);
    m_renderGraph.addNewAttachments(m_alphaDeferredPass, m_positionAttachments[1]);
    m_renderGraph.addNewAttachments(m_alphaDeferredPass, m_normalAttachments[1]);
    m_renderGraph.addNewAttachments(m_alphaDeferredPass, m_rmtAttachments[1]);
    //m_renderGraph.transferAttachmentsToAttachments(m_alphaDetectPass, m_alphaDeferredPass, 1);
    m_renderGraph.transferAttachmentsToAttachments(m_deferredPass, m_alphaDeferredPass, 4);

    m_renderGraph.transferAttachmentsToUniforms(m_alphaDetectPass, m_alphaDeferredPass, 0); //This contains opac position in xyz and truly trans in w
    //m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_alphaDeferredPass, 1);
}

void SceneRenderer::prepareSsgiBNRenderPasses()
{
    /// compute bent normals
    m_ssgiBNPass = m_renderGraph.addRenderPass();

    m_renderGraph.addNewAttachments(m_ssgiBNPass, m_ssgiAccuBentNormalsAttachment,
                                    VK_ATTACHMENT_STORE_OP_STORE/*, VK_ATTACHMENT_LOAD_OP_LOAD*/);

    for(size_t i = 0 ; i < NBR_SSGI_SAMPLES ; ++i)
        m_renderGraph.addNewAttachments(m_ssgiBNPass, m_ssgiCollisionsAttachments[i]);

    m_renderGraph.transferAttachmentsToUniforms(m_alphaDetectPass, m_ssgiBNPass, 0); //Position
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ssgiBNPass, 2); //Normal

    /// blur horizontal
    m_ssgiBNBlurPasses[0] = m_renderGraph.addRenderPass();

    m_renderGraph.addNewAttachments(m_ssgiBNBlurPasses[0], m_SSGIBlurBentNormalsAttachments[0]);
    m_renderGraph.transferAttachmentsToUniforms(m_ssgiBNPass        , m_ssgiBNBlurPasses[0], 0);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDetectPass   , m_ssgiBNBlurPasses[0], 0); //Position
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass , m_ssgiBNBlurPasses[0], 1); //Position

    /// blur vertical
    m_ssgiBNBlurPasses[1] = m_renderGraph.addRenderPass();

    m_renderGraph.addNewAttachments(m_ssgiBNBlurPasses[1], m_SSGIBlurBentNormalsAttachments[1]);
    m_renderGraph.transferAttachmentsToUniforms(m_ssgiBNBlurPasses[0], m_ssgiBNBlurPasses[1], 0);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDetectPass    , m_ssgiBNBlurPasses[1], 0); //Position
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass  , m_ssgiBNBlurPasses[1], 1); //Position

}

void SceneRenderer::prepareLightingRenderPass()
{
    m_lightingPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderGraph.addNewAttachments(m_lightingPass, m_hdrAttachements[0]);
    //m_renderGraph.addNewAttachments(m_lightingPass, m_hdrAttachements[1]);

    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_lightingPass, 0);
    //m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_lightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDetectPass, m_lightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_lightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_lightingPass, 3);

    /*m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_lightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_lightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_lightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_lightingPass, 3);*/

    m_renderGraph.transferAttachmentsToUniforms(m_ssgiBNBlurPasses[1], m_lightingPass, 0);
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

    m_renderGraph.transferAttachmentsToUniforms(m_ssgiBNBlurPasses[1], m_alphaLightingPass, 0);

    //m_renderGraph.addNewUniforms(m_alphaLightingPass, m_ambientLightingUbo);
}

void SceneRenderer::prepareSsgiLightingRenderPass()
{

}

void SceneRenderer::prepareAmbientLightingRenderPass()
{
    m_ambientLightingPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    //m_renderGraph.addNewAttachments(m_ambientLightingPass, m_hdrAttachements[0]);
    //m_renderGraph.addNewAttachments(m_ambientLightingPass, m_hdrAttachements[1]);

    m_renderGraph.transferAttachmentsToAttachments(m_lightingPass, m_ambientLightingPass, 0);
    //m_renderGraph.transferAttachmentsToAttachments(m_lightingPass, m_ambientLightingPass, 1);
    m_renderGraph.transferAttachmentsToAttachments(m_alphaLightingPass, m_ambientLightingPass, 0);

    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 0);
    //m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDetectPass, m_ambientLightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_deferredPass, m_ambientLightingPass, 3);

    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_ambientLightingPass, 0);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_ambientLightingPass, 1);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_ambientLightingPass, 2);
    m_renderGraph.transferAttachmentsToUniforms(m_alphaDeferredPass, m_ambientLightingPass, 3);

    m_renderGraph.transferAttachmentsToUniforms(m_ssgiBNBlurPasses[1], m_ambientLightingPass, 0);

   // m_renderGraph.addNewUniforms(m_ambientLightingPass, m_ambientLightingUbo);

    size_t texturesCount = /*m_targetWindow->getFramesCount();*/m_targetWindow->getSwapchainSize();
    std::vector<VkImageView> brdflut(texturesCount, PBRToolbox::getBrdflut().view);
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

bool SceneRenderer::createSpriteShadowsGenPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_SHADOWGEN_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_SHADOWGEN_FRAGSHADERFILE;

    m_spriteShadowsGenPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_spriteShadowsGenPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_spriteShadowsGenPipeline.addSpecializationDatum(uint32_t(128), 1); //Nbr ray steps
    m_spriteShadowsGenPipeline.addSpecializationDatum(uint32_t(8), 1); //Nbr search steps
    m_spriteShadowsGenPipeline.addSpecializationDatum(0.02f, 1); //Ray collision thresold

    auto bindingDescription = SpriteShadowGenerationDatum::getBindingDescription();
    auto attributeDescriptions = SpriteShadowGenerationDatum::getAttributeDescriptions();
    m_spriteShadowsGenPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_spriteShadowsGenPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_spriteShadowsGenPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    return m_spriteShadowsGenPipeline.init(m_renderGraph.getRenderPass(m_spriteShadowsPass));
}

bool SceneRenderer::createSpriteShadowsPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_SHADOW_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_SHADOW_FRAGSHADERFILE;

    m_spriteShadowsPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_spriteShadowsPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = IsoSpriteShadowDatum::getBindingDescription();
    auto attributeDescriptions = IsoSpriteShadowDatum::getAttributeDescriptions();
    m_spriteShadowsPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_spriteShadowsPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);

    m_spriteShadowsPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_spriteShadowsPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_spriteShadowsPipeline.attachPushConstant(VK_SHADER_STAGE_VERTEX_BIT , sizeof(glm::vec4));

    m_spriteShadowsPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER);

    return m_spriteShadowsPipeline.init(m_renderGraph.getRenderPass(m_shadowMapsPass));

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

    //m_deferredSpritesPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent(), true);

    m_deferredSpritesPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_deferredSpritesPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_deferredSpritesPipeline.attachPushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4));

    m_deferredSpritesPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER);

    return m_deferredSpritesPipeline.init(m_renderGraph.getRenderPass(m_deferredPass));
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

    //m_deferredMeshesPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent(), true);

    m_deferredMeshesPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_deferredMeshesPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_deferredMeshesPipeline.attachPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::vec4));

    m_deferredMeshesPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER);

    m_deferredMeshesPipeline.setCullMode(VK_CULL_MODE_BACK_BIT);

    return m_deferredMeshesPipeline.init(m_renderGraph.getRenderPass(m_deferredPass));
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

    //m_alphaDetectPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent(), true);

    m_alphaDetectPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_alphaDetectPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_alphaDetectPipeline.attachPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::vec4));


    m_alphaDetectPipeline.setWriteMask(VK_COLOR_COMPONENT_A_BIT);

   /// m_alphaDetectPipeline.setDepthTest(false, true, VK_COMPARE_OP_GREATER);

    /*VkStencilOpState stencil = {};
    stencil.compareOp   = VK_COMPARE_OP_ALWAYS;
	stencil.failOp      = VK_STENCIL_OP_REPLACE;
	stencil.depthFailOp = VK_STENCIL_OP_REPLACE;
	stencil.passOp      = VK_STENCIL_OP_REPLACE;
	stencil.compareMask = 0xff;
	stencil.writeMask   = 0xff;
    stencil.reference   = 1;
    m_alphaDetectPipeline.setStencilTest(true, stencil);*/

    return m_alphaDetectPipeline.init(m_renderGraph.getRenderPass(m_alphaDetectPass));
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

    //m_alphaDeferredPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent(), true);

    m_alphaDeferredPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_alphaDeferredPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());
    m_alphaDeferredPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_alphaDeferredPass));

    m_alphaDeferredPipeline.attachPushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4));

    m_alphaDeferredPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER_OR_EQUAL);

   /* VkStencilOpState stencil = {};
    stencil.compareOp   = VK_COMPARE_OP_EQUAL;//VK_COMPARE_OP_EQUAL;
	stencil.failOp      = VK_STENCIL_OP_KEEP;
	stencil.depthFailOp = VK_STENCIL_OP_KEEP;
	stencil.passOp      = VK_STENCIL_OP_REPLACE;
	stencil.compareMask = 0xff;
	stencil.writeMask   = 0xff;
    stencil.reference   = 1;
    m_alphaDeferredPipeline.setStencilTest(true, stencil);*/

    VkStencilOpState stencil = {};
    stencil.compareOp   = VK_COMPARE_OP_ALWAYS;
	stencil.failOp      = VK_STENCIL_OP_REPLACE;
	stencil.depthFailOp = VK_STENCIL_OP_REPLACE;//VK_STENCIL_OP_ZERO;
	stencil.passOp      = VK_STENCIL_OP_REPLACE;
	stencil.compareMask = 0xff;
	stencil.writeMask   = 0xff;
    stencil.reference   = 1;
    m_alphaDeferredPipeline.setStencilTest(true, stencil);

    return m_alphaDeferredPipeline.init(m_renderGraph.getRenderPass(m_alphaDeferredPass));
}

bool SceneRenderer::createSsgiBNPipelines()
{
    ///Bent normals computation
    {
        std::ostringstream vertShaderPath,fragShaderPath;
        vertShaderPath << VApp::DEFAULT_SHADERPATH << SSGI_BN_VERTSHADERFILE;
        fragShaderPath << VApp::DEFAULT_SHADERPATH << SSGI_BN_FRAGSHADERFILE;

        m_ssgiBNPipeline.addSpecializationDatum(15.0f, 1); //Ray length
        m_ssgiBNPipeline.addSpecializationDatum(1.0f, 1); //Ray length factor
        m_ssgiBNPipeline.addSpecializationDatum(20.0f, 1); //Ray threshold
        m_ssgiBNPipeline.addSpecializationDatum(2.0f, 1); //Ray threshold factor

        m_ssgiBNPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
        m_ssgiBNPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

        m_ssgiBNPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent());

        m_ssgiBNPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
        m_ssgiBNPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_ssgiBNPass));

        m_ssgiBNPipeline.attachPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(int));

       // m_ssgiBNPipeline.setBlendMode(BlendMode_Accu, 0);

        if(!m_ssgiBNPipeline.init(m_renderGraph.getRenderPass(m_ssgiBNPass)))
            return (false);
    }

    ///Horizontal and vertical blur
    for(size_t i = 0 ; i < 2 ; ++i)
    {
        std::ostringstream vertShaderPath,fragShaderPath;
        vertShaderPath << VApp::DEFAULT_SHADERPATH << BLUR_VERTSHADERFILE;
        fragShaderPath << VApp::DEFAULT_SHADERPATH << BLUR_FRAGSHADERFILE;

        m_ssgiBNBlurPipelines[i].createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
        m_ssgiBNBlurPipelines[i].createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

        //m_ssgiBNBlurPipelines[i].setSpecializationInfo(specializationInfo, 1);

        m_ssgiBNBlurPipelines[i].addSpecializationDatum(4.0f ,1); //Radius
        m_ssgiBNBlurPipelines[i].addSpecializationDatum(15.0f,1); //Smart thresold
        m_ssgiBNBlurPipelines[i].addSpecializationDatum(static_cast<bool>(i),1); //Vertical

        m_ssgiBNBlurPipelines[i].setStaticExtent(m_targetWindow->getSwapchainExtent());

        m_ssgiBNBlurPipelines[i].attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_ssgiBNBlurPasses[i]));

        if(!m_ssgiBNBlurPipelines[i].init(m_renderGraph.getRenderPass(m_ssgiBNBlurPasses[i])))
            return (false);
    }


    return (true);
}

bool SceneRenderer::createLightingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << LIGHTING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << LIGHTING_FRAGSHADERFILE;

    m_lightingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_lightingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_lightingPipeline.addSpecializationDatum(2.0f, 1); //SSAO intensity
    m_lightingPipeline.addSpecializationDatum(4.0f, 1); //GIAO intensity

    auto bindingDescription = LightDatum::getBindingDescription();
    auto attributeDescriptions = LightDatum::getAttributeDescriptions();
    m_lightingPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_lightingPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, false);

    //m_lightingPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent(), true);

    m_lightingPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_lightingPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());
    m_lightingPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_lightingPass));
    //m_deferredSpritesPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_lightingPipeline.attachPushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4));

    m_lightingPipeline.setBlendMode(BlendMode_Add,0);
    //m_lightingPipeline.setBlendMode(BlendMode_Add,1);

    return m_lightingPipeline.init(m_renderGraph.getRenderPass(m_lightingPass));
}

bool SceneRenderer::createAlphaLightingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << LIGHTING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << LIGHTING_FRAGSHADERFILE;

    m_alphaLightingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_alphaLightingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_alphaLightingPipeline.addSpecializationDatum(2.0f, 1); //SSAO intensity
    m_alphaLightingPipeline.addSpecializationDatum(4.0f, 1); //GIAO intensity

    auto bindingDescription = LightDatum::getBindingDescription();
    auto attributeDescriptions = LightDatum::getAttributeDescriptions();
    m_alphaLightingPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_alphaLightingPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, false);

    //m_alphaLightingPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent(), true);

    m_alphaLightingPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_alphaLightingPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());
    m_alphaLightingPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_alphaLightingPass));
    //m_deferredSpritesPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_alphaLightingPipeline.attachPushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4));

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

    return m_alphaLightingPipeline.init(m_renderGraph.getRenderPass(m_alphaLightingPass));
}

bool SceneRenderer::createSsgiLightingPipeline()
{
    return (true);
}

bool SceneRenderer::createAmbientLightingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << AMBIENTLIGHTING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << AMBIENTLIGHTING_FRAGSHADERFILE;

    m_ambientLightingPipeline.addSpecializationDatum(PBRToolbox::ENVMAP_FILTERINGMIPSCOUNT, 1);

    m_ambientLightingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_ambientLightingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    //m_ambientLightingPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent(), true);

    m_ambientLightingPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_ambientLightingPass));
    m_ambientLightingPipeline.attachDescriptorSetLayout(SceneRenderingData::ambientLightingDescSetLayout());
    //m_ambientLightingPipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());

    m_ambientLightingPipeline.attachPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4));

    m_ambientLightingPipeline.setBlendMode(BlendMode_Add,0);
    m_ambientLightingPipeline.setBlendMode(BlendMode_Add,1);

    return m_ambientLightingPipeline.init(m_renderGraph.getRenderPass(m_ambientLightingPass));
}

bool SceneRenderer::createToneMappingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << TONEMAPPING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << TONEMAPPING_FRAGSHADERFILE;

    m_toneMappingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_toneMappingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_toneMappingPipeline.setStaticExtent(m_targetWindow->getSwapchainExtent());

    m_toneMappingPipeline.setBlendMode(BlendMode_None);

    //m_toneMappingPipeline.attachDescriptorSetLayout(m_hdrDescriptorSetLayout);
    m_toneMappingPipeline.attachDescriptorSetLayout(m_renderGraph.getDescriptorLayout(m_toneMappingPass));

    return m_toneMappingPipeline.init(m_renderGraph.getRenderPass(m_toneMappingPass));
}

void SceneRenderer::cleanup()
{
    for(auto vbo : m_spriteShadowGenerationVbos)
        delete vbo;
    m_spriteShadowGenerationVbos.clear();

    for(auto vbo : m_spriteShadowsVbos)
        delete vbo;
    m_spriteShadowsVbos.clear();

    for(auto vbo : m_spritesVbos)
        delete vbo;
    m_spritesVbos.clear();

    for(auto meshVboMap : m_meshesVbos)
        for(auto meshVbo : meshVboMap)
            delete meshVbo.second;
    m_meshesVbos.clear();

    for(auto attachement : m_deferredDepthAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_deferredDepthAttachments.clear();

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

    VulkanHelpers::destroyAttachment(m_ssgiAccuBentNormalsAttachment);
    VulkanHelpers::destroyAttachment(m_ssgiAccuLightingAttachment);
    for(size_t i = 0 ; i < NBR_SSGI_SAMPLES ; ++i)
        VulkanHelpers::destroyAttachment(m_ssgiCollisionsAttachments[i]);
    for(size_t i = 0 ; i < 2 ; ++i)
        VulkanHelpers::destroyAttachment(m_SSGIBlurBentNormalsAttachments[i]);

    m_spriteShadowsGenPipeline.destroy();
    m_spriteShadowsPipeline.destroy();
    m_deferredSpritesPipeline.destroy();
    m_deferredMeshesPipeline.destroy();
    m_alphaDetectPipeline.destroy();
    m_alphaDeferredPipeline.destroy();
    m_ssgiBNPipeline.destroy();
    m_lightingPipeline.destroy();
    m_ssgiLightingPipeline.destroy();
    m_ambientLightingPipeline.destroy();
    m_toneMappingPipeline.destroy();

    SceneRenderingData::cleanStatic();

    AbstractRenderer::cleanup();
}


}
