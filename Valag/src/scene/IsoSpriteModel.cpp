#include "Valag/scene/IsoSpriteModel.h"

#include "Valag/assets/MaterialAsset.h"
#include "Valag/assets/AssetHandler.h"

namespace vlg
{

IsoSpriteModel::IsoSpriteModel() :
    m_material(0),
    m_size({1.0f,1.0f}),
    m_texturePosition({0.0f,0.0f}),
    m_textureExtent({1.0f,1.0f}),
    m_textureCenter({0.0f,0.0f})
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
        if(m_material != nullptr)
            this->stopListeningTo(m_material);
        m_material = material;
        if(m_material != nullptr)
            this->startListeningTo(m_material);
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

void IsoSpriteModel::cleanup()
{

}

void IsoSpriteModel::notify(NotificationSender *sender, NotificationType notification)
{
    if(notification == Notification_AssetLoaded)
        this->sendNotification(Notification_ModelChanged);
    if(notification == Notification_SenderDestroyed)
    {
        if(sender == m_material)
        {
            m_material = nullptr;
            this->sendNotification(Notification_ModelChanged);
        }
    }
}


}
