#ifndef SCENENODE_H
#define SCENENODE_H

#include <list>
#include <set>
#include <glm/glm.hpp>

#include "Valag/Types.h"
#include "Valag/scene/SceneEntity.h"
#include "Valag/scene/LightEntity.h"
#include "Valag/scene/ShadowCaster.h"

#include "Valag/core/NotificationListener.h"
#include "Valag/core/NotificationSender.h"


namespace vlg{

class Scene;
class SceneRenderer;

class SceneNode : public NotificationSender, public NotificationListener
{
    public:
        SceneNode(const NodeTypeId);
        SceneNode(const NodeTypeId, Scene* scene);
        virtual ~SceneNode();

        void addChildNode(SceneNode*);
        void addChildNode(const NodeTypeId id, SceneNode*);

        SceneNode* removeChildNode(SceneNode*);
        SceneNode* removeChildNode(const NodeTypeId id);

        SceneNode* createChildNode();
        SceneNode* createChildNode(float, float );
        SceneNode* createChildNode(float, float, float );
        SceneNode* createChildNode(glm::vec2 );
        SceneNode* createChildNode(glm::vec3 );
        SceneNode* createChildNode(const NodeTypeId id);

        bool destroyChildNode(SceneNode*);
        bool destroyChildNode(const NodeTypeId id);

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

        void scale(float scale);
        void scale(glm::vec3 scale);
        void setScale(float scale);
        void setScale(glm::vec3 scale);
        void rotate(float value, glm::vec3 axis);
        void setRotation(glm::vec3 rotation);

        /**setRotation and co**/

        glm::vec3 getGlobalPosition();
        glm::vec3 getScale();
        glm::vec3 getEulerRotation();
        const glm::mat4 &getModelMatrix();

        glm::vec3 getPosition();

        /**sf::FloatRect getGlobalBounds();
        sf::FloatRect getBounds();**/

        NodeTypeId getId();
        SceneNode* getParent();
        Scene*  getScene();

        //void searchInsideForEntities(std::list<SceneEntity*>  *renderQueue);

        /**void SearchInsideForEntities(std::list<SceneEntity*>  *renderQueue);

        void FindNearbyLights(std::multimap<float, Light*> *foundedLights);
        void SearchInsideForLights(std::multimap<float, Light*> *foundedLights, sf::Vector3f);

        void FindNearbyShadowCaster(std::list<ShadowCaster*> *foundedCaster, LightType);
        void SearchInsideForShadowCaster(std::list<ShadowCaster*> *foundedCaster, LightType);**/

        void update(const Time &elapsedTime);

        //void render(SceneRenderer *renderer);
        void generateRenderingData(SceneRenderingInstance *renderingInstance);

        virtual void notify(NotificationSender* , NotificationType,
                            size_t dataSize = 0, char* data = nullptr) override;

    protected:
        void setParent(SceneNode *);
        void setScene(Scene *);
        void setId(const NodeTypeId );
        NodeTypeId generateId();

        void updateGlobalPosition();
        void updateModelMatrix();

    protected:
        glm::vec3 m_globalPosition;

        glm::vec3 m_position;
        glm::vec3 m_eulerRotations;
        glm::vec3 m_scale;

        glm::mat4 m_modelMatrix;

        Scene* m_scene;

    private:
        NodeTypeId m_id;
        SceneNode *m_parent;
        std::map<NodeTypeId, SceneNode*> m_childs;
        std::set<NodeTypeId> m_createdChildsList;

        std::list<SceneObject *>    m_attachedObjects;
        std::list<SceneEntity *>    m_attachedEntities;
        std::list<LightEntity *>    m_attachedLights;
        std::list<ShadowCaster *>   m_attachedShadowCasters;

        int m_curNewId;
};

}

#endif // SCENENODE_H
