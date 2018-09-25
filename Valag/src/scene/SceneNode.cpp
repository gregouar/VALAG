#include "Valag/scene/SceneNode.h"

#include "Valag/scene/Scene.h"
#include "Valag/scene/SceneObject.h"
#include "Valag/utils/Logger.h"


namespace vlg
{

SceneNode::SceneNode(const NodeTypeID id) : SceneNode(id, nullptr)
{
}

SceneNode::SceneNode(const NodeTypeID id, SceneNode *p) :
    m_globalPosition(0.0,0.0,0.0),
    m_position(0.0,0.0,0.0),
    m_eulerRotations(0.0,0.0,0.0),
    m_scale(1.0,1.0,1.0)
{
    m_scene = nullptr;
    m_parent = p;

    if(m_parent != nullptr)
        this->setScene(m_parent->getScene());

    m_id = id;
    m_curNewId = 0;
}

SceneNode::SceneNode(const NodeTypeID id, SceneNode *p, Scene* scene) :
    SceneNode(id,p)
{
    this->setScene(scene);
}

SceneNode::~SceneNode()
{
    this->removeAndDestroyAllChilds();
}


void SceneNode::addChildNode(SceneNode* node)
{
    NodeTypeID id = this->generateID();
    this->addChildNode(id, node);
    if(node != nullptr)
        node->setID(id);
}

void SceneNode::addChildNode(const NodeTypeID id, SceneNode* node)
{
    auto childsIt = m_childs.find(id);
    if(childsIt != m_childs.end())
    {
        std::ostringstream warn_report;
        warn_report << "Adding child of same id as another one (ID="<<id<<")";
        Logger::warning(warn_report);
    }

    m_childs[id] = node;

    if(node != nullptr)
    {
        node->setScene(m_scene);
        this->askForAllNotifications(node);
    }

    if(m_scene != nullptr)
        m_scene->askToComputeRenderQueue();
}

SceneNode* SceneNode::removeChildNode(const NodeTypeID id)
{
    SceneNode* node = nullptr;

    auto childsIt = m_childs.find(id);

    if(childsIt == m_childs.end())
    {
        std::ostringstream error_report;
        error_report << "Cannot remove child node (ID="<<id<<")";
        Logger::error(error_report);

        return (nullptr);
    }

    node = childsIt->second;

    this->removeFromAllNotificationList(node);

    /*std::list<NodeTypeID>::iterator createdChildsIt;
    createdChildsIt = std::list::find(m_createdChildsList.begin(),
                                m_createdChildsList.end(), id);

    if(createdChildsIt != m_createdChildsList.end())*/
    if(m_createdChildsList.find(id) != m_createdChildsList.end())
        Logger::warning("Removing created child without destroying it");

    m_childs.erase(childsIt);

    if(m_scene != nullptr)
        m_scene->askToComputeRenderQueue();

    return node;
}

SceneNode* SceneNode::removeChildNode(SceneNode* node)
{
    if(node != nullptr && node->getParent() == this)
        return this->removeChildNode(node->getID());
    return (nullptr);
}

SceneNode* SceneNode::createChildNode()
{
    return this->createChildNode(this->generateID());
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

SceneNode* SceneNode::createChildNode(const NodeTypeID id)
{
    auto childsIt = m_childs.find(id);
    if(childsIt != m_childs.end())
    {
        std::ostringstream error_report;
        error_report << "Cannot create new child node with the same ID as an existing child node (ID="<<id<<")";
        Logger::error(error_report);

        return childsIt->second;
    }

    SceneNode* newNode = new SceneNode(id, this);
    m_createdChildsList.insert(id);

    this->addChildNode(id, newNode);

    return newNode;
}

bool SceneNode::destroyChildNode(SceneNode* node)
{
    if(node != nullptr && node->getParent() == this)
        return this->destroyChildNode(node->getID());
    return (false);
}

bool SceneNode::destroyChildNode(const NodeTypeID id)
{
    /*std::list<NodeTypeID>::iterator createdChildsIt;
    createdChildsIt = std::find(m_createdChildsList.begin(),
                                m_createdChildsList.end(), id);*/

    auto createdChildsIt = m_createdChildsList.find(id);

    if(createdChildsIt == m_createdChildsList.end())
        Logger::warning("Destroying non-created child");
    else
        m_createdChildsList.erase(createdChildsIt);


    auto childsIt = m_childs.find(id);

    if(childsIt == m_childs.end())
    {
        std::ostringstream error_report;
        error_report << "Cannot destroy child (ID="<<id<<")";
        Logger::error(error_report);

        return (false);
    }

    if(childsIt->second != nullptr)
        delete childsIt->second;
    this->removeChildNode(id);

    return (true);
}

void SceneNode::removeAndDestroyAllChilds(bool destroyNonCreatedChilds)
{
    //std::map<NodeTypeID, SceneNode*>::iterator childsIt;

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

    if(m_scene != nullptr)
        m_scene->askToComputeRenderQueue();
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
            m_entities.push_back(dynamic_cast<SceneEntity*>(e));

        /**if(e->IsALight())
            m_lights.push_back((Light*)e);

        if(e->IsAShadowCaster())
            //m_shadowCasters.push_back(dynamic_cast<ShadowCaster*>(e));
            m_shadowCasters.push_back((ShadowCaster*)e);**/

        if(e->setParentNode(this) != nullptr)
            Logger::warning("Attaching entity which has already a parent node");

        this->askForAllNotifications(e);
    } else
        Logger::error("Cannot attach null entity");

    if(m_scene != nullptr)
        m_scene->askToComputeRenderQueue();
}

void SceneNode::detachObject(SceneObject *e)
{
    m_attachedObjects.remove(e);

    if(e != nullptr && e->isAnEntity())
        m_entities.remove(dynamic_cast<SceneEntity*>(e));

    /**if(e != nullptr && e->isALight())
        m_lights.remove((Light*)e);

    if(e != nullptr && e->isAShadowCaster())
        m_shadowCasters.remove((ShadowCaster*)e);**/

    this->removeFromAllNotificationList(e);
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

    if(m_parent != nullptr)
        m_globalPosition = m_parent->getGlobalPosition() + pos;
    else
        m_globalPosition = pos;

    if(m_scene != nullptr)
        m_scene->askToComputeRenderQueue();

    this->sendNotification(Notification_SceneNodeMoved);
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

    /**Update childs global pos**/

    this->sendNotification(Notification_SceneNodeMoved);
}

void SceneNode::rotate(float value, glm::vec3 axis)
{
    this->setRotation(m_eulerRotations+value*axis);
}

void SceneNode::setRotation(glm::vec3 rotation)
{
    m_eulerRotations = rotation;
    /**Update childs global pos**/

    this->sendNotification(Notification_SceneNodeMoved);
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

NodeTypeID SceneNode::getID()
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



void SceneNode::setID(const NodeTypeID id)
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
    m_parent = p;
}

NodeTypeID SceneNode::generateID()
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

void SceneNode::render(SceneRenderer *renderer)
{
    for(auto entity : m_entities)
        if(entity->isVisible())
            entity->draw(renderer);

    for(auto node : m_childs)
        node.second->render(renderer);
}

void SceneNode::notify(NotificationSender* sender, NotificationType type)
{
    /** Auto Update Global Position Here ETC **/
    if(sender == m_parent)
    {
        if(type == Notification_SceneNodeMoved)
            this->setPosition(m_position);
    }
}

}
