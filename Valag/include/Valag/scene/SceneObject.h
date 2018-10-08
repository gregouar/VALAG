#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "Valag/Types.h"
#include "Valag/core/NotificationListener.h"

/** Any object that can be put in a scene : instance of sprite or mesh, ligth source, camera, ambient sound, ... **/

namespace vlg
{

class SceneNode;

class SceneObject : public NotificationListener
{
    friend class SceneNode;

    public:
        SceneObject();
        virtual ~SceneObject();

        SceneNode *getParentNode();

        bool isALight();
        bool isAnEntity();
        bool isAShadowCaster();

        virtual void update(const Time &elapsedTime);
        virtual void notify(NotificationSender*, NotificationType);

    protected:
        SceneNode *setParentNode(SceneNode*);
        SceneNode *m_parentNode;

        bool m_isALight;
        bool m_isAnEntity;
        bool m_isAShadowCaster;

    private:
};

}

#endif // SCENEOBJECT_H
