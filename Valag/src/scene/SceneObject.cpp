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
    m_isVisible = true;
}

SceneObject::~SceneObject()
{
    //dtor
}


SceneNode* SceneObject::setParentNode(SceneNode *newParent)
{
    SceneNode* oldParent = getParentNode();
    m_parentNode = newParent;
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

bool SceneObject::isVisible()
{
    return m_isVisible;
}

void SceneObject::setVisible(bool visible)
{
    m_isVisible = visible;
}


void SceneObject::update(const Time &elapsedTime)
{

}

void SceneObject::notify(NotificationSender* sender, NotificationType type)
{
    if(sender == (NotificationSender*)m_parentNode)
    {
        if(type == Notification_SceneNodeDetroyed)
            m_parentNode = nullptr;
    }
}


}
