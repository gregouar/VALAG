#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "Valag/core/NotificationListener.h"
#include "Valag/Types.h"

namespace vlg
{

class SceneNode;
/**class SceneEntity;
class Light;**/

class SceneObject : public NotificationListener
{
    friend class SceneNode;
   /** friend class SceneEntity; //SceneEntity::SceneEntity();
    friend class GeometricShadowCaster; //SceneEntity::SceneEntity();
    friend class Light;//::Light();**/

    public:
        SceneObject();
        virtual ~SceneObject();

        SceneNode *getParentNode();

        virtual bool isALight();
        virtual bool isAnEntity();
        virtual bool isAShadowCaster();

        virtual void update(const Time &elapsedTime);

        virtual void notify(NotificationSender*, NotificationType);

        bool isVisible();
        void setVisible(bool = true);

    protected:
        SceneNode *setParentNode(SceneNode*);
        SceneNode *m_parentNode;

    private:
        bool m_isALight;
        bool m_isAnEntity;
        bool m_isAShadowCaster;
        bool m_isVisible;
};

}

#endif // SCENEOBJECT_H
