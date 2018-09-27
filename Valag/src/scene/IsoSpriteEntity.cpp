#include "Valag/scene/IsoSpriteEntity.h"

#include "Valag/assets/AssetHandler.h"
#include "Valag/assets/MaterialAsset.h"
#include "Valag/renderers/SceneRenderer.h"
#include "Valag/scene/SceneNode.h"

namespace vlg
{


VkVertexInputBindingDescription IsoSpriteDatum::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(IsoSpriteDatum);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 12> IsoSpriteDatum::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 12> attributeDescriptions = {};

    size_t i = 0;
    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, position);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, rotation);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, size);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, center);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, albedo_color);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, rmt_color);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, texPos);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, texExtent);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, albedo_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, height_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, normal_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(IsoSpriteDatum, rmt_texId);
    ++i;


    return attributeDescriptions;
}


IsoSpriteEntity::IsoSpriteEntity() :
    m_spriteModel(nullptr),
    m_rotation(0.0f),
    m_color(1.0,1.0,1.0,1.0),
    m_rmt(1.0,1.0,1.0)
{
    this->updateDatum();
}

IsoSpriteEntity::~IsoSpriteEntity()
{
    //dtor
}

void IsoSpriteEntity::setRotation(float rotation)
{
    if(m_rotation != rotation)
    {
        m_rotation = rotation;
        this->updateDatum();
    }
}


void IsoSpriteEntity::setColor(Color color)
{
    if(m_color != color)
    {
        m_color = color;
        this->updateDatum();
    }
}

void IsoSpriteEntity::setRmt(glm::vec3 rmt)
{
    if(m_rmt != rmt)
    {
        m_rmt = rmt;
        this->updateDatum();
    }
}

void IsoSpriteEntity::setSpriteModel(IsoSpriteModel* model)
{
    if(m_spriteModel != model)
    {
        if(m_spriteModel != nullptr)
            this->stopListeningTo(m_spriteModel);
        m_spriteModel = model;
        if(m_spriteModel != nullptr)
            this->startListeningTo(m_spriteModel);
        this->updateDatum();
    }
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

IsoSpriteDatum IsoSpriteEntity::getIsoSpriteDatum()
{
    return m_datum;
}


void IsoSpriteEntity::draw(SceneRenderer *renderer)
{
    renderer->addToSpritesVbo(this->getIsoSpriteDatum());
}

void IsoSpriteEntity::notify(NotificationSender *sender, NotificationType notification)
{
    if(notification == Notification_AssetLoaded ||
       notification == Notification_TextureChanged ||
       notification == Notification_ModelChanged ||
       notification == Notification_SceneNodeMoved)
        this->updateDatum();

    if(notification == Notification_SenderDestroyed)
    {
        if(sender == m_spriteModel)
        {
            m_spriteModel = nullptr;
            this->updateDatum();
        }
    }
}

void IsoSpriteEntity::updateDatum()
{
    m_datum = {};
    if(m_spriteModel == nullptr || m_parentNode == nullptr)
        return;

    float heightFactor = 1.0;

    m_datum.albedo_color = m_color;
    m_datum.rmt_color = m_rmt;

    MaterialAsset* material = m_spriteModel->getMaterial();
    if(material != nullptr && material->isLoaded())
    {
        m_datum.albedo_texId = {material->getAlbedoMap().m_textureId,
                              material->getAlbedoMap().m_textureLayer};
        m_datum.height_texId = {material->getHeightMap().m_textureId,
                              material->getHeightMap().m_textureLayer};
        m_datum.normal_texId = {material->getNormalMap().m_textureId,
                              material->getNormalMap().m_textureLayer};
        m_datum.rmt_texId    = {material->getRmtMap().m_textureId,
                              material->getRmtMap().m_textureLayer};

        heightFactor = material->getHeightFactor();
        m_datum.rmt_color *= material->getRmtFactor();
    }

    m_datum.position = m_parentNode->getGlobalPosition();
    m_datum.size = glm::vec3(m_spriteModel->getSize(), heightFactor);
    m_datum.rotation = m_rotation;
    m_datum.center = m_spriteModel->getTextureCenter();

    m_datum.texPos    = m_spriteModel->getTexturePosition();
    m_datum.texExtent = m_spriteModel->getTextureExtent();
}


}
