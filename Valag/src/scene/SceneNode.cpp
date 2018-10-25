#include "Valag/scene/SceneNode.h"

#include "Valag/scene/Scene.h"
#include "Valag/scene/SceneObject.h"
#include "Valag/utils/Logger.h"


namespace vlg
{

SceneNode::SceneNode(const NodeTypeId id) :
    m_globalPosition(0.0,0.0,0.0),
    m_position(0.0,0.0,0.0),
    m_eulerRotations(0.0,0.0,0.0),
    m_scale(1.0,1.0,1.0),
    m_modelMatrix(1.0)
{
    m_scene = nullptr;
    m_parent = nullptr;
    m_id = id;
    m_curNewId = 0;
}

SceneNode::SceneNode(const NodeTypeId id, Scene* scene) :
    SceneNode(id)
{
    this->setScene(scene);
}

SceneNode::~SceneNode()
{
    this->removeAndDestroyAllChilds();
}


void SceneNode::addChildNode(SceneNode* node)
{
    NodeTypeId id = this->generateId();
    this->addChildNode(id, node);
    if(node != nullptr)
        node->setId(id);
}

void SceneNode::addChildNode(const NodeTypeId id, SceneNode* node)
{
    auto childsIt = m_childs.find(id);
    if(childsIt != m_childs.end())
    {
        std::ostringstream warn_report;
        warn_report << "Adding child of same id as another one (Id="<<id<<")";
        Logger::warning(warn_report);
    }

    m_childs[id] = node;
    node->setParent(this);

    /*if(node != nullptr)
    {
        node->setScene(m_scene);
        this->askForAllNotifications(node);
    }*/

    //if(m_scene != nullptr)
      //  m_scene->askToComputeRenderQueue();
}

SceneNode* SceneNode::removeChildNode(const NodeTypeId id)
{
    SceneNode* node = nullptr;

    auto childsIt = m_childs.find(id);

    if(childsIt == m_childs.end())
    {
        std::ostringstream error_report;
        error_report << "Cannot remove child node (Id="<<id<<")";
        Logger::error(error_report);

        return (nullptr);
    }

    node = childsIt->second;
    node->setParent(nullptr);

    //this->removeFromAllNotificationList(node);

    if(m_createdChildsList.find(id) != m_createdChildsList.end())
        Logger::warning("Removing created child without destroying it");

    m_childs.erase(childsIt);

    //if(m_scene != nullptr)
      //  m_scene->askToComputeRenderQueue();

    return node;
}

SceneNode* SceneNode::removeChildNode(SceneNode* node)
{
    if(node != nullptr && node->getParent() == this)
        return this->removeChildNode(node->getId());
    return (nullptr);
}

SceneNode* SceneNode::createChildNode()
{
    return this->createChildNode(this->generateId());
}

SceneNode* SceneNode::createChildNode(float x, float y, float z)
{
    SceneNode* newNode = this->createChildNode();
    if(newNode != nullptr)
        newNode->setPosition(x,y,z);
    return newNode;
}


SceneNode* SceneNode::createChildNode(float x, float y)
{
    return this->createChildNode(x,y,0);
}

SceneNode* SceneNode::createChildNode(glm::vec2 p)
{
    return this->createChildNode(p.x, p.y);
}

SceneNode* SceneNode::createChildNode(glm::vec3 p)
{
    return this->createChildNode(p.x, p.y, p.z);
}

SceneNode* SceneNode::createChildNode(const NodeTypeId id)
{
    auto childsIt = m_childs.find(id);
    if(childsIt != m_childs.end())
    {
        std::ostringstream error_report;
        error_report << "Cannot create new child node with the same Id as an existing child node (Id="<<id<<")";
        Logger::error(error_report);

        return childsIt->second;
    }

    SceneNode* newNode = new SceneNode(id);
    m_createdChildsList.insert(id);

    this->addChildNode(id, newNode);

    return newNode;
}

bool SceneNode::destroyChildNode(SceneNode* node)
{
    if(node != nullptr && node->getParent() == this)
        return this->destroyChildNode(node->getId());
    return (false);
}

bool SceneNode::destroyChildNode(const NodeTypeId id)
{
    auto createdChildsIt = m_createdChildsList.find(id);

    if(createdChildsIt == m_createdChildsList.end())
        Logger::warning("Destroying non-created child");
    else
        m_createdChildsList.erase(createdChildsIt);


    auto childsIt = m_childs.find(id);

    if(childsIt == m_childs.end())
    {
        std::ostringstream error_report;
        error_report << "Cannot destroy child (Id="<<id<<")";
        Logger::error(error_report);

        return (false);
    }

    if(childsIt->second != nullptr)
        delete childsIt->second;
    m_childs.erase(childsIt);
    //this->removeChildNode(id);

    return (true);
}

void SceneNode::removeAndDestroyAllChilds(bool destroyNonCreatedChilds)
{
    //std::map<NodeTypeId, SceneNode*>::iterator childsIt;

    if(!destroyNonCreatedChilds)
        while(!m_createdChildsList.empty())
            this->destroyChildNode(*m_createdChildsList.begin());

    if(destroyNonCreatedChilds)
    {
        //childsIt = m_childs.begin();
        //for(;childsIt != m_childs.end() ; ++childsIt)
        for(auto childsIt : m_childs)
        {
            if(childsIt.second != nullptr)
            {
                childsIt.second->removeAndDestroyAllChilds(destroyNonCreatedChilds);
                delete childsIt.second;
            }
        }
    }

    m_childs.clear();
    m_createdChildsList.clear();

    //if(m_scene != nullptr)
      //  m_scene->askToComputeRenderQueue();
}

/**SceneNodeIterator SceneNode::GetChildIterator()
{
    return SceneNodeIterator(m_childs.begin(), m_childs.end());
}

SceneObjectIterator SceneNode::GetSceneObjectIterator()
{
    return SceneObjectIterator(m_attachedObjects.begin(), m_attachedObjects.end());
}**/


void SceneNode::attachObject(SceneObject *e)
{
    if(e != nullptr)
    {
        m_attachedObjects.push_back(e);

        if(e->isAnEntity())
            m_attachedEntities.push_back(dynamic_cast<SceneEntity*>(e));

        if(e->isALight())
            m_attachedLights.push_back(dynamic_cast<LightEntity*>(e));

        if(e->isAShadowCaster())
            m_attachedShadowCasters.push_back(dynamic_cast<ShadowCaster*>(e));

        if(e->setParentNode(this) != nullptr)
            Logger::warning("Attaching entity which has already a parent node");

        e->notify(this, Notification_SceneNodeMoved);
    } else
        Logger::error("Cannot attach null entity");

    //if(m_scene != nullptr)
      //  m_scene->askToComputeRenderQueue();
}

void SceneNode::detachObject(SceneObject *e)
{
    if(e->getParentNode() != this)
    {
        Logger::warning("Tried to detach object from non-parent node");
        return;
    }

    m_attachedObjects.remove(e);

    if(e != nullptr && e->isAnEntity())
        m_attachedEntities.remove(dynamic_cast<SceneEntity*>(e));

    if(e != nullptr && e->isALight())
        m_attachedLights.remove(dynamic_cast<LightEntity*>(e));

    if(e != nullptr && e->isAShadowCaster())
        m_attachedShadowCasters.remove(dynamic_cast<ShadowCaster*>(e));

    e->setParentNode(nullptr);
}

/**SceneEntityIterator SceneNode::GetEntityIterator()
{
    return SceneEntityIterator(m_entities.begin(), m_entities.end());
}

LightIterator SceneNode::GetLightIterator()
{
    return LightIterator(m_lights.begin(), m_lights.end());
}

ShadowCasterIterator SceneNode::GetShadowCasterIterator()
{
    return ShadowCasterIterator(m_shadowCasters.begin(), m_shadowCasters.end());
}**/

void SceneNode::move(float x, float y)
{
    this->move(x,y,0);
}

void SceneNode::move(float x, float y, float z)
{
    this->move(glm::vec3(x,y,z));
}

void SceneNode::move(glm::vec2 p)
{
    this->move(glm::vec3(p.x,p.y,0));
}

void SceneNode::move(glm::vec3 p)
{
    glm::vec3 newPos = this->getPosition();
    newPos += p;
    this->setPosition(newPos);
}


void SceneNode::setPosition(float x, float y)
{
    this->setPosition(glm::vec2(x,y));
}

void SceneNode::setPosition(float x, float y, float z)
{
    this->setPosition(glm::vec3(x, y, z));
}

void SceneNode::setPosition(glm::vec2 xyPos)
{
    this->setPosition(glm::vec3(xyPos.x, xyPos.y,this->getPosition().z));
}

void SceneNode::setPosition(glm::vec3 pos)
{
    m_position = pos;

    this->updateGlobalPosition();

   /* if(m_parent != nullptr)
        m_globalPosition = m_parent->getGlobalPosition() + pos;
    else
        m_globalPosition = pos;

    if(m_scene != nullptr)
        m_scene->askToComputeRenderQueue();

    this->sendNotification(Notification_SceneNodeMoved);*/
}

void SceneNode::scale(float scale)
{
    this->scale({scale, scale, scale});
}

void SceneNode::scale(glm::vec3 scale)
{
    this->setScale(m_scale*scale);
}

void SceneNode::setScale(float scale)
{
    this->setScale({scale, scale, scale});
}

void SceneNode::setScale(glm::vec3 scale)
{
    m_scale = scale;
    this->updateModelMatrix();
}

void SceneNode::rotate(float value, glm::vec3 axis)
{
    this->setRotation(m_eulerRotations+value*axis);
}

void SceneNode::setRotation(glm::vec3 rotation)
{
    m_eulerRotations = rotation;
    this->updateModelMatrix();
}

glm::vec3 SceneNode::getPosition()
{
    return m_position;
}

glm::vec3 SceneNode::getGlobalPosition()
{
    return m_globalPosition;
}

glm::vec3 SceneNode::getScale()
{
    return m_scale;
}

glm::vec3 SceneNode::getEulerRotation()
{
    return m_eulerRotations;
}

const glm::mat4 &SceneNode::getModelMatrix()
{
    return m_modelMatrix;
}

NodeTypeId SceneNode::getId()
{
    return m_id;
}

Scene* SceneNode::getScene()
{
    return m_scene;
}

SceneNode* SceneNode::getParent()
{
    return m_parent;
}



void SceneNode::setId(const NodeTypeId id)
{
    m_id = id;
}

void SceneNode::setScene(Scene *scene)
{
    m_scene = scene;

    for(auto node : m_childs)
        node.second->setScene(scene);

    /**SceneNodeIterator childIt = GetChildIterator();
    while(!childIt.IsAtTheEnd())
    {
        SceneNode *curChild = childIt.GetElement();
        if(curChild != nullptr)
            curChild->setSceneManager(sceneManager);
        ++childIt;
    }**/
}

void SceneNode::setParent(SceneNode *p)
{
    if(m_parent != p)
    {
        this->stopListeningTo(m_parent);
        m_parent = p;
        if(m_parent != nullptr)
        {
            this->setScene(m_parent->getScene());
            this->startListeningTo(m_parent);
        }
        this->updateGlobalPosition();
    }
}

NodeTypeId SceneNode::generateId()
{
    return m_curNewId++;
}


/**void SceneNode::searchInsideForEntities(std::list<SceneEntity*>  *renderQueue)
{
    if(renderQueue != nullptr)
    {
        SceneEntityIterator entityIt = GetEntityIterator();
        while(!entityIt.IsAtTheEnd())
        {
           // if(entityIt.GetElement()->IsRenderable())
            if(entityIt.GetElement()->IsVisible())
                renderQueue->push_back(entityIt.GetElement());
            ++entityIt;
        }

        SceneNodeIterator nodeIt = GetChildIterator();
        while(!nodeIt.IsAtTheEnd())
        {
            nodeIt.GetElement()->SearchInsideForEntities(renderQueue);
            ++nodeIt;
        }
    }
}


void SceneNode::FindNearbyLights(std::multimap<float,Light*> *foundedLights)
{
    GetSceneManager()->GetRootNode()->SearchInsideForLights(foundedLights, GetGlobalPosition());
}

void SceneNode::SearchInsideForLights(std::multimap<float,Light*> *foundedLights, sf::Vector3f pos)
{
    if(foundedLights != nullptr)
    {
        LightIterator lightIt = GetLightIterator();
        while(!lightIt.IsAtTheEnd())
        {
            if(lightIt.GetElement()->IsVisible())
            {
                float distance = -1; //To put directional lights at the front of the list

                if(lightIt.GetElement()->GetType() != DirectionnalLight)
                    distance = SquareDistance(pos,GetGlobalPosition());

                foundedLights->insert(std::pair<float, Light*>(distance,lightIt.GetElement()));
            }

            ++lightIt;
        }

        SceneNodeIterator nodeIt = GetChildIterator();
        while(!nodeIt.IsAtTheEnd())
        {
            nodeIt.GetElement()->SearchInsideForLights(foundedLights, pos);
            ++nodeIt;
        }
    }
}


void SceneNode::FindNearbyShadowCaster(std::list<ShadowCaster*> *foundedCaster, LightType lightType)
{
    GetSceneManager()->GetRootNode()->SearchInsideForShadowCaster(foundedCaster, lightType);
}

void SceneNode::SearchInsideForShadowCaster(std::list<ShadowCaster*> *foundedCaster, LightType lightType)
{
    if(foundedCaster != nullptr)
    {
        ShadowCasterIterator casterIt = GetShadowCasterIterator();
        while(!casterIt.IsAtTheEnd())
        {
            if(casterIt.GetElement()->IsVisible())
                if(casterIt.GetElement()->GetShadowCastingType() == AllShadows
                || (lightType == DirectionnalLight
                    && casterIt.GetElement()->GetShadowCastingType() == DirectionnalShadow)
                || (lightType == OmniLight
                    && casterIt.GetElement()->GetShadowCastingType() == DynamicShadow))
                    foundedCaster->push_back(casterIt.GetElement());
            ++casterIt;
        }

        SceneNodeIterator nodeIt = GetChildIterator();
        while(!nodeIt.IsAtTheEnd())
        {
            nodeIt.GetElement()->SearchInsideForShadowCaster(foundedCaster, lightType);
            ++nodeIt;
        }
    }
}**/

void SceneNode::update(const Time &elapsedTime)
{
    /**SceneObjectIterator objIt = GetSceneObjectIterator();
    while(!objIt.IsAtTheEnd())
    {
        objIt.GetElement()->Update(elapsedTime);
        ++objIt;
    }

    SceneNodeIterator nodeIt = GetChildIterator();
    while(!nodeIt.IsAtTheEnd())
    {
        nodeIt.GetElement()->Update(elapsedTime);
        ++nodeIt;
    }**/
}

void SceneNode::updateGlobalPosition()
{
    if(m_parent != nullptr)
    {
        glm::vec4 pos    = m_parent->getModelMatrix() * glm::vec4(m_position,1.0);
        m_globalPosition = {pos.x,pos.y,pos.z};
    }
    else
        m_globalPosition = m_position;

    //if(m_scene != nullptr)
      //  m_scene->askToComputeRenderQueue();

    this->updateModelMatrix();
   // this->sendNotification(Notification_SceneNodeMoved);
}

void SceneNode::updateModelMatrix()
{
    m_modelMatrix = glm::translate(glm::mat4(1.0), m_globalPosition);
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.x, glm::vec3(1.0,0.0,0.0));
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.y, glm::vec3(0.0,1.0,0.0));
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.z, glm::vec3(0.0,0.0,1.0));
    m_modelMatrix = glm::scale(m_modelMatrix, m_scale);

    this->sendNotification(Notification_SceneNodeMoved);
}

/*void SceneNode::render(SceneRenderer *renderer)
{
    for(auto entity : m_attachedEntities)
        if(entity->isVisible())
            entity->draw(renderer);

    for(auto node : m_childs)
        node.second->render(renderer);
}*/

void SceneNode::generateRenderingData(SceneRenderingInstance *renderingInstance)
{
    for(auto entity : m_attachedEntities)
        if(entity->isVisible())
            entity->generateRenderingData(renderingInstance);

    for(auto light : m_attachedLights)
        if(light->isVisible())
            renderingInstance->addToShadowLightsList(light);

    for(auto shadowCaster : m_attachedShadowCasters)
        if(shadowCaster->isVisible())
            renderingInstance->addToShadowCastersList(shadowCaster);

    for(auto node : m_childs)
        node.second->generateRenderingData(renderingInstance);
}

void SceneNode::notify(NotificationSender* sender, NotificationType type,
                       size_t dataSize, char* data)
{
    /** Auto Update Global Position Here ETC **/
    if(sender == m_parent)
    {
        if(type == Notification_SceneNodeMoved)
            this->updateGlobalPosition();
        if(type == Notification_SenderDestroyed)
        {
            m_parent = nullptr;
            this->updateGlobalPosition();
        }
    }
}

}
