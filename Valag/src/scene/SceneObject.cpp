#include "Valag/scene/SceneObject.h"

#include "Valag/scene/SceneNode.h"

namespace vlg
{


SceneObject::SceneObject()
{
    m_parentNode        = nullptr;
    m_isALight          = false;
    m_isAnEntity        = false;
    m_isAShadowCaster   = false;
}

SceneObject::~SceneObject()
{
    //dtor
}


SceneNode* SceneObject::setParentNode(SceneNode *newParent)
{
    SceneNode* oldParent = getParentNode();

    if(m_parentNode != newParent)
    {
        this->stopListeningTo(m_parentNode);
        m_parentNode = newParent;
        this->startListeningTo(m_parentNode);
    }
    return oldParent;
}

SceneNode* SceneObject::getParentNode()
{
    return m_parentNode;
}


bool SceneObject::isALight()
{
    return m_isALight;
}

bool SceneObject::isAnEntity()
{
    return m_isAnEntity;
}

bool SceneObject::isAShadowCaster()
{
    return m_isAShadowCaster;
}


void SceneObject::update(const Time &elapsedTime)
{

}

void SceneObject::notify(NotificationSender* sender, NotificationType type,
                         size_t dataSize, char* data)
{
    if(sender == (NotificationSender*)m_parentNode)
    {
        if(type == Notification_SceneNodeDetroyed)
            m_parentNode = nullptr;
    }
}


}
