#ifndef SCENENODE_H
#define SCENENODE_H

#include <list>
#include <set>
#include <glm/glm.hpp>

#include "Valag/Types.h"
///#include "Valag/gfx/SceneEntity.h"

#include "Valag/core/NotificationListener.h"
#include "Valag/core/NotificationSender.h"


namespace vlg{

class Scene;

class SceneNode : public NotificationSender, public NotificationListener
{
    public:
        SceneNode(const NodeTypeID&);
        SceneNode(const NodeTypeID&, SceneNode* parent);
        SceneNode(const NodeTypeID&, SceneNode* parent, Scene* scene);
        virtual ~SceneNode();

        void addChildNode(SceneNode*);
        void addChildNode(const NodeTypeID &id, SceneNode*);

        SceneNode* removeChildNode(SceneNode*);
        SceneNode* removeChildNode(const NodeTypeID &id);

        SceneNode* createChildNode();
        SceneNode* createChildNode(float, float );
        SceneNode* createChildNode(float, float, float );
        SceneNode* createChildNode(glm::vec2 );
        SceneNode* createChildNode(glm::vec3 );
        SceneNode* createChildNode(const NodeTypeID &id);

        bool destroyChildNode(SceneNode*);
        bool destroyChildNode(const NodeTypeID &id);

        void removeAndDestroyAllChilds(bool destroyNonCreatedChilds = false);

        /**SceneNodeIterator   getChildIterator();
        SceneObjectIterator getSceneObjectIterator();
        SceneEntityIterator getEntityIterator();
        LightIterator GetLightIterator();
        ShadowCasterIterator GetShadowCasterIterator();**/

        void attachObject(SceneObject *);
        void detachObject(SceneObject *);
        void detachAllObjects();

        void move(float, float);
        void move(float, float, float);
        void move(glm::vec2 );
        void move(glm::vec3 );
        void setPosition(float, float);
        void setPosition(float, float, float);
        void setPosition(glm::vec2 );
        void setPosition(glm::vec3 );

        glm::vec3 getGlobalPosition();
        glm::vec3 getPosition();

        /**sf::FloatRect getGlobalBounds();
        sf::FloatRect getBounds();**/

        const NodeTypeID& getID();
        SceneNode* getParent();
        Scene*  getScene();

        /**void SearchInsideForEntities(std::list<SceneEntity*>  *renderQueue);

        void FindNearbyLights(std::multimap<float, Light*> *foundedLights);
        void SearchInsideForLights(std::multimap<float, Light*> *foundedLights, sf::Vector3f);

        void FindNearbyShadowCaster(std::list<ShadowCaster*> *foundedCaster, LightType);
        void SearchInsideForShadowCaster(std::list<ShadowCaster*> *foundedCaster, LightType);**/

        void update(const Time &elapsedTime);

        virtual void notify(NotificationSender*, NotificationType);

    protected:
        void setParent(SceneNode *);
        void setScene(Scene *);
        void setID(const NodeTypeID &);
        NodeTypeID generateID();

        glm::vec3 m_position;
        glm::vec3 m_globalPosition;

        Scene* m_scene;

    private:
        NodeTypeID m_id;
        SceneNode *m_parent;
        std::map<NodeTypeID, SceneNode*> m_childs;
        std::set<NodeTypeID> m_createdChildsList;

        std::list<SceneObject *> m_attachedObjects;
        /**std::list<SceneEntity *> m_entities;
        std::list<Light *> m_lights;
        std::list<ShadowCaster *> m_shadowCasters;**/

        int m_curNewId;
};

}

#endif // SCENENODE_H
