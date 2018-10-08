#include "Valag/scene/ShadowCaster.h"

namespace vlg
{

ShadowCaster::ShadowCaster() : SceneEntity()
{
    m_isAShadowCaster = true;
}

ShadowCaster::~ShadowCaster()
{
    //dtor
}

}
