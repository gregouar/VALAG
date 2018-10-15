#include "Valag/scene/ShadowCaster.h"

namespace vlg
{

ShadowCaster::ShadowCaster() : SceneEntity(),
    m_shadowCastingType(ShadowCasting_None)
{
    m_isAShadowCaster = true;
}

ShadowCaster::~ShadowCaster()
{
    //dtor
}

void ShadowCaster::setShadowCasting(ShadowCastingType type)
{
    m_shadowCastingType = type;
}

ShadowCastingType ShadowCaster::getShadowCastingType()
{
    return m_shadowCastingType;
}

}
