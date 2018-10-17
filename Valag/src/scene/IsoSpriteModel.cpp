#include "Valag/scene/IsoSpriteModel.h"

#include <Vulkan/vulkan.hpp>

#include "Valag/assets/MaterialAsset.h"
#include "Valag/assets/AssetHandler.h"

namespace vlg
{

IsoSpriteModel::IsoSpriteModel() :
    m_material(0),
    m_size({1.0f,1.0f}),
    m_texturePosition({0.0f,0.0f}),
    m_textureExtent({1.0f,1.0f}),
    m_textureCenter({0.0f,0.0f}),
    m_isReady(true),
    m_shadowMapExtent(256.0,256.0)
{
    //ctor
}

IsoSpriteModel::~IsoSpriteModel()
{
    this->cleanup();
}

void IsoSpriteModel::setMaterial(AssetTypeId materialId)
{
    this->setMaterial(MaterialsHandler::instance()->getAsset(materialId));
}

void IsoSpriteModel::setMaterial(MaterialAsset *material)
{
    if(m_material != material)
    {
        this->stopListeningTo(m_material);
        m_material = material;
        this->startListeningTo(m_material);

        if(m_material != nullptr && !m_material->isLoaded())
            m_isReady = false;
        else
            m_isReady = true;

        this->sendNotification(Notification_ModelChanged);
    }
}

void IsoSpriteModel::setSize(glm::vec2 size)
{
    if(m_size != size)
    {
        m_size = size;
        this->sendNotification(Notification_ModelChanged);
    }
}

void IsoSpriteModel::setTextureRect(glm::vec2 pos, glm::vec2 extent)
{
    if(m_texturePosition != pos || m_textureExtent != extent)
    {
        m_texturePosition = pos;
        m_textureExtent = extent;
        this->sendNotification(Notification_ModelChanged);
    }
}

void IsoSpriteModel::setTextureCenter(glm::vec2 pos)
{
    if(m_textureCenter != pos)
    {
        m_textureCenter = pos;
        this->sendNotification(Notification_ModelChanged);
    }
}

void IsoSpriteModel::setColor(Color color)
{
    if(m_color != color)
    {
        m_color = color;
        this->sendNotification(Notification_ModelChanged);
    }
}

void IsoSpriteModel::setRmt(Color rmt)
{
    if(m_rmt != rmt)
    {
        m_rmt = rmt;
        this->sendNotification(Notification_ModelChanged);
    }
}


MaterialAsset* IsoSpriteModel::getMaterial()
{
    return m_material;
}

glm::vec2 IsoSpriteModel::getSize()
{
    return m_size;
}

glm::vec2 IsoSpriteModel::getTextureExtent()
{
    return m_textureExtent;
}

glm::vec2 IsoSpriteModel::getTexturePosition()
{
    return m_texturePosition;
}

glm::vec2 IsoSpriteModel::getTextureCenter()
{
    return m_textureCenter;
}

bool IsoSpriteModel::isReady()
{
    return m_isReady;
}

VTexture IsoSpriteModel::getDirectionnalShadow(glm::vec3 direction)
{
    auto foundedMap = m_directionnalShadows.find(direction);
    if(foundedMap == m_directionnalShadows.end())
        return this->generateDirectionnalShadow(direction);
    return foundedMap->second.texture;
}

/// Protected ///

void IsoSpriteModel::cleanup()
{
    for(auto shadowMap : m_directionnalShadows)
        VTexturesManager::freeTexture(shadowMap.second);
    m_directionnalShadows.clear();
}

void IsoSpriteModel::notify(NotificationSender *sender, NotificationType notification)
{
    if(notification == Notification_AssetLoaded)
    {
        m_isReady = true;
        this->sendNotification(Notification_ModelChanged);
    }
    if(notification == Notification_SenderDestroyed)
    {
        if(sender == m_material)
        {
            m_material = nullptr;
            this->sendNotification(Notification_ModelChanged);
        }
    }
}

VTexture IsoSpriteModel::generateDirectionnalShadow(glm::vec3 direction)
{
    VRenderableTexture *renderableTexture = &m_directionnalShadows[direction];
    VTexturesManager::allocRenderableTexture(m_shadowMapExtent.x, m_shadowMapExtent.y, VK_FORMAT_R8G8B8A8_UNORM,
                                             renderableTexture);

    /*do something*/

    return renderableTexture->texture;
}


}
