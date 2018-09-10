#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "Valag/Types.h"
#include "Valag/core/NotificationListener.h"

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

        bool isVisible();
        void setVisible(bool = true);

        virtual void update(const Time &elapsedTime);
        virtual void notify(NotificationSender*, NotificationType);

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
