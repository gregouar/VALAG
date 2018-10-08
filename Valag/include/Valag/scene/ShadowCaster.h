#ifndef SHADOWCASTER_H
#define SHADOWCASTER_H

#include "Valag/scene/SceneEntity.h"

namespace vlg
{

class Light;

class ShadowCaster : public SceneEntity
{
    public:
        ShadowCaster();
        virtual ~ShadowCaster();

        void addLightSource();

    protected:

    protected:
        std::set<Light*> m_lightSources;
};

}

#endif // SHADOWCASTER_H
