#include "Valag/renderers/SceneRenderingInstance.h"

#include "Valag/renderers/SceneRenderer.h"

namespace vlg
{


/// SceneRenderingInstance ///

SceneRenderingInstance::SceneRenderingInstance(SceneRenderingData *data, SceneRenderer *renderer) :
    m_renderer(renderer),
    m_renderingData(data),
    m_spritesVboSize(0),
    m_lightsVboSize(0)
{
}

void SceneRenderingInstance::addToSpritesVbo(const IsoSpriteDatum &datum)
{
    if(m_spritesVboSize == 0)
        m_spritesVboOffset = m_renderer->getSpritesVboSize();
    m_renderer->addToSpritesVbo(datum);
    ++m_spritesVboSize;
}

void SceneRenderingInstance::addToMeshesVbo(VMesh *mesh, const MeshDatum &datum)
{
    auto foundedSize = m_meshesVboSize.find(mesh);
    if(foundedSize == m_meshesVboSize.end())
    {
        foundedSize = m_meshesVboSize.insert(foundedSize, {mesh,0});
        m_meshesVboOffset[mesh] = m_renderer->getMeshesVboSize(mesh);
    }
    m_renderer->addToMeshesVbo(mesh, datum);
    ++foundedSize->second;
}

void SceneRenderingInstance::addToLightsVbo(const LightDatum &datum)
{
    if(m_lightsVboSize == 0)
        m_lightsVboOffset = m_renderer->getLightsVboSize();
    m_renderer->addToLightsVbo(datum);
    ++m_lightsVboSize;
}

void SceneRenderingInstance::addToShadowLightsList(LightEntity *entity)
{
    if(entity->isCastingShadows())
        m_renderedShadowLights.push_back(entity);
}

void SceneRenderingInstance::addToShadowCastersList(ShadowCaster *caster)
{
    if(caster->getShadowCastingType() == ShadowCasting_All
    || caster->getShadowCastingType() == ShadowCasting_OnlyDynamic)
        m_renderedShadowCasters[0].push_back(caster);

    if(caster->getShadowCastingType() == ShadowCasting_All
    || caster->getShadowCastingType() == ShadowCasting_OnlyDirectionnal)
        m_renderedShadowCasters[1].push_back(caster);
}

void SceneRenderingInstance::setViewInfo(const ViewInfo &viewInfo, glm::vec3 camPos, float camZoom)
{
    m_viewInfo  = viewInfo;
    m_camPos    = camPos;
    m_camZoom   = camZoom;
}

SceneRenderingData *SceneRenderingInstance::getRenderingData()
{
    return m_renderingData;
}

size_t SceneRenderingInstance::getSpritesVboSize()
{
    return m_spritesVboSize;
}

size_t SceneRenderingInstance::getSpritesVboOffset()
{
    return m_spritesVboOffset;
}

size_t SceneRenderingInstance::getMeshesVboSize(VMesh *mesh)
{
    auto founded = m_meshesVboSize.find(mesh);
    if(founded == m_meshesVboSize.end())
        return (0);
    return founded->second;
}

size_t SceneRenderingInstance::getMeshesVboOffset(VMesh *mesh)
{
    auto founded = m_meshesVboOffset.find(mesh);
    if(founded == m_meshesVboOffset.end())
        return (0);
    return founded->second;
}

size_t SceneRenderingInstance::getLightsVboSize()
{
    return m_lightsVboSize;
}

size_t SceneRenderingInstance::getLightsVboOffset()
{
    return m_lightsVboOffset;
}

const ViewInfo &SceneRenderingInstance::getViewInfo()
{
    return m_viewInfo;
}

glm::vec4 SceneRenderingInstance::getCamPosAndZoom()
{
    return glm::vec4(m_camPos, m_camZoom);
}

//Could add option for shader stage and offset
void SceneRenderingInstance::pushCamPosAndZoom(VkCommandBuffer cmb, VkPipelineLayout layout, VkFlags shaderStageBit)
{
    glm::vec4 pc = this->getCamPosAndZoom();
    vkCmdPushConstants(cmb, layout,
        shaderStageBit, 0, sizeof(glm::vec4), (void*)&pc);
}


}
