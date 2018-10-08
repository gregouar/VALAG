#include "Valag/scene/SceneEntity.h"

namespace vlg
{

SceneEntity::SceneEntity() :
    m_isVisible(true)
{
    m_isAnEntity = true;
}

SceneEntity::~SceneEntity()
{
    //dtor
}

bool SceneEntity::isVisible()
{
    return m_isVisible;
}

void SceneEntity::setVisible(bool visible)
{
    m_isVisible = visible;
}

}
