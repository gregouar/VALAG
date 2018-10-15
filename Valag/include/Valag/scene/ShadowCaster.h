#ifndef SHADOWCASTER_H
#define SHADOWCASTER_H

#include "Valag/scene/SceneEntity.h"

namespace vlg
{

class LightEntity;
class SceneRenderer;

class ShadowCaster : public SceneEntity
{
    public:
        ShadowCaster();
        virtual ~ShadowCaster();

        //void addLightSource();

        void setShadowCasting(ShadowCastingType type);

        ShadowCastingType getShadowCastingType();

        virtual void castShadow(SceneRenderer *renderer, LightEntity* light) = 0;

        //virtual void notify(NotificationSender*, NotificationType);

    protected:

    protected:
        ShadowCastingType m_shadowCastingType;

       // std::set<Light*> m_lightSources;
};

}

#endif // SHADOWCASTER_H
