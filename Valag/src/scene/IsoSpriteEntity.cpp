#include "Valag/scene/IsoSpriteEntity.h"

#include "Valag/assets/AssetHandler.h"
#include "Valag/assets/MaterialAsset.h"
#include "Valag/renderers/SceneRenderer.h"
#include "Valag/scene/SceneNode.h"

namespace vlg
{


VkVertexInputBindingDescription InstanciedIsoSpriteDatum::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(InstanciedIsoSpriteDatum);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 12> InstanciedIsoSpriteDatum::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 12> attributeDescriptions = {};

    size_t i = 0;
    /**attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, model_0);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, model_1);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, model_2);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, model_3);
    ++i;**/

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, position);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, rotation);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, size);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, center);
    ++i;




    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, albedo_color);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, rmt_color);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, texPos);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, texExtent);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, albedo_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, height_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, normal_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, rmt_texId);
    ++i;


    return attributeDescriptions;
}


IsoSpriteEntity::IsoSpriteEntity() :
    m_spriteModel(nullptr),
    m_rotation(0.0f),
    m_color(1.0,1.0,1.0,1.0),
    m_rmt(1.0,1.0,1.0)
{
    //ctor
}

IsoSpriteEntity::~IsoSpriteEntity()
{
    //dtor
}

void IsoSpriteEntity::setRotation(float rotation)
{
    m_rotation = rotation;
}


void IsoSpriteEntity::setColor(Color color)
{
    m_color = color;
}

void IsoSpriteEntity::setRmt(glm::vec3 rmt)
{
    m_rmt = rmt;
}

void IsoSpriteEntity::setSpriteModel(IsoSpriteModel* model)
{
    m_spriteModel = model;
}


float IsoSpriteEntity::getRotation()
{
    return m_rotation;
}

Color IsoSpriteEntity::getColor()
{
    return m_color;
}

glm::vec3 IsoSpriteEntity::getRmt()
{
    return m_rmt;
}

///I'll need to improve modelMatrix (update only when necessary)
InstanciedIsoSpriteDatum IsoSpriteEntity::getIsoSpriteDatum()
{
    InstanciedIsoSpriteDatum datum;

    datum.albedo_texId = {0,0};
    datum.height_texId = {0,0};
    datum.normal_texId = {0,0};
    datum.rmt_texId    = {0,0};

    MaterialAsset* material = MaterialsHandler::instance()->getAsset(m_spriteModel->getMaterial());
    if(material->isLoaded())
    {
        datum.albedo_texId = {material->getAlbedoMap().m_textureId,
                              material->getAlbedoMap().m_textureLayer};
        datum.height_texId = {material->getHeightMap().m_textureId,
                              material->getHeightMap().m_textureLayer};
        datum.normal_texId = {material->getNormalMap().m_textureId,
                              material->getNormalMap().m_textureLayer};
        datum.rmt_texId    = {material->getRmtMap().m_textureId,
                              material->getRmtMap().m_textureLayer};
    }

    /**glm::vec2 scale = m_spriteModel->getSize();


    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0), m_parentNode->getGlobalPosition());
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-m_spriteModel->getTextureCenter(), 0.0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, material->getHeightFactor()));

    datum.model_0 = modelMatrix[0];
    datum.model_1 = modelMatrix[1];
    datum.model_2 = modelMatrix[2];
    datum.model_3 = modelMatrix[3];**/

    datum.position = m_parentNode->getGlobalPosition();
    datum.size = glm::vec3(m_spriteModel->getSize(), material->getHeightFactor());
    datum.rotation = m_rotation;
    datum.center = m_spriteModel->getTextureCenter();

    datum.albedo_color = m_color;
    datum.rmt_color = m_rmt * material->getRmtFactor();
    datum.texPos    = m_spriteModel->getTexturePosition();
    datum.texExtent = m_spriteModel->getTextureExtent();

    return datum;
}


void IsoSpriteEntity::draw(SceneRenderer *renderer)
{
    renderer->addToSpritesVbo(this->getIsoSpriteDatum());
}


/** Static **/
/*DynamicUBODescriptor IsoSpriteEntity::s_entityUBO(sizeof(IsoSpriteEntityUBO), 4096);

bool IsoSpriteEntity::initRendering(size_t framesCount)
{
    return IsoSpriteEntity::s_entityUBO.init(framesCount) & IsoSpriteModel::s_modelUBO.init(framesCount);
}

void IsoSpriteEntity::updateRendering(size_t frameIndex)
{
    IsoSpriteModel::s_modelUBO.update(frameIndex);
    IsoSpriteEntity::s_entityUBO.update(frameIndex);
}

void IsoSpriteEntity::cleanupRendering()
{
    IsoSpriteModel::s_modelUBO.cleanup();
    IsoSpriteEntity::s_entityUBO.cleanup();
}

VkDescriptorSetLayout IsoSpriteEntity::getUBODescriptorSetLayout()
{
    return IsoSpriteEntity::s_entityUBO.getDescriptorSetLayout();
}*/


}
