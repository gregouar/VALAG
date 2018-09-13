#include "Valag/scene/Scene.h"

#include "Valag/utils/Logger.h"
#include "Valag/scene/SceneObject.h"

namespace vlg
{

///const sf::Vector2u Scene::DEFAULT_SHADOWMAP_SIZE = sf::Vector2u(1024,1024);


Scene::Scene() :
    m_rootNode(0,nullptr, this)
{
    m_rootNode.setPosition(0,0,0);
    m_curNewId = 0;
    m_needToUpdateRenderQueue = false;
    ///m_last_target = nullptr;
    ///m_currentCamera = nullptr;

    ///m_ambientLight = sf::Color::White;
    ///m_shadowCastingOption = NoShadow;
    ///m_enableSRGB = false;
}

Scene::~Scene()
{
    this->cleanAll();
}

void Scene::cleanAll()
{
    m_rootNode.removeAndDestroyAllChilds();
    this->destroyAllCreatedObjects();
}


/**bool Scene::initRenderer(sf::Vector2u windowSize)
{
    return (true);
}**/

void Scene::update(const Time &elapsedTime)
{
    m_rootNode.update(elapsedTime);

    /**if(m_needToUpdateRenderQueue)
    {
        this->computeRenderQueue();
        m_needToUpdateRenderQueue = false;
    }**/
}

void Scene::askToComputeRenderQueue()
{
    m_needToUpdateRenderQueue = true;
}

/**void Scene::ProcessRenderQueue(sf::RenderTarget *w)
{
    std::list<SceneEntity*>::iterator renderIt;
    for(renderIt = m_renderQueue.begin() ; renderIt != m_renderQueue.end(); ++renderIt)
        RenderEntity(w,*renderIt);
}

void Scene::RenderEntity(sf::RenderTarget *w, SceneEntity *entity)
{
    sf::RenderStates state;
    state.transform = sf::Transform::Identity;

    sf::Vector3f globalPos(0,0,0);

    SceneNode *node = entity->GetParentNode();
    if(node != nullptr)
        globalPos = node->GetGlobalPosition();

    state.transform.translate(globalPos.x, globalPos.y);

    entity->Render(w,state);
}

void Scene::ComputeRenderQueue()
{
    m_renderQueue.clear();
    //m_staticRenderQueue.clear();
    //AddToRenderQueue(&m_rootNode);
    m_rootNode.SearchInsideForEntities(&m_renderQueue);
}


int Scene::UpdateLighting(std::multimap<float, Light*> &lightList, int maxNbrLights)
{
    if(maxNbrLights == -1)
        maxNbrLights = Light::GetMaxNbrLights();

    int curNbrLights = 0;

    std::multimap<float, Light*>::iterator lightIt;
    for(lightIt = lightList.begin() ; lightIt != lightList.end() && curNbrLights < maxNbrLights ; ++lightIt)
    {
        Light* curLight = lightIt->second;
        SceneNode* node = curLight->GetParentNode();

        if(node != nullptr)
        {
            sf::Vector3f pos(0,0,0);
            GLfloat glPos[] = { 0,0,0,1.0 };
            GLfloat glDirection[] = { 0,0,1 };
            GLfloat glColor[] = {1,1,1,1};

            if(curLight->GetType() == DirectionnalLight)
            {
                pos = curLight->GetDirection();
                glPos[3] = 0;
            } else
                pos = node->GetGlobalPosition();

            glPos[0] = pos.x;
            glPos[1] = pos.y;
            glPos[2] = pos.z;

            glLightfv(GL_LIGHT0+curNbrLights, GL_POSITION, glPos);
            SfColorToGlColor(curLight->GetDiffuseColor(), glColor);
            glColor[0] *= curLight->GetIntensity();
            glColor[1] *= curLight->GetIntensity();
            glColor[2] *= curLight->GetIntensity();
            glLightfv(GL_LIGHT0+curNbrLights, GL_DIFFUSE, glColor);
           // glLightf(GL_LIGHT0+curNbrLights, GL_CONSTANT_ATTENUATION, curLight->GetConstantAttenuation());
            glLightf(GL_LIGHT0+curNbrLights, GL_CONSTANT_ATTENUATION, 1.0/(curLight->GetRadius()*0.01));
            glLightf(GL_LIGHT0+curNbrLights, GL_LINEAR_ATTENUATION, curLight->GetLinearAttenuation());
            glLightf(GL_LIGHT0+curNbrLights, GL_QUADRATIC_ATTENUATION, curLight->GetQuadraticAttenuation());

            pos = curLight->GetDirection();
            glDirection[0] = pos.x;
            glDirection[1] = pos.y;
            glDirection[2] = pos.z;
            glLightfv(GL_LIGHT0+curNbrLights, GL_SPOT_DIRECTION, glDirection);

            if(m_shadowCastingOption != NoShadow
            && curLight->IsCastShadowEnabled())
            if(m_shadowCastingOption == AllShadows
            || (curLight->GetType() == DirectionnalLight
                && m_shadowCastingOption == DirectionnalShadow)
            || (curLight->GetType() == OmniLight
                && m_shadowCastingOption == DynamicShadow))
            {
                curLight->GetShadowCasterList()->clear();
                node->FindNearbyShadowCaster(curLight->GetShadowCasterList(),curLight->GetType());
                curLight->UpdateShadow();
            }

            ++curNbrLights;
        }
    }

    return curNbrLights;
}



void Scene::RenderShadows(std::multimap<float, Light*> &lightList,const sf::View &view,
                                 int maxNbrLights)
{
    if(maxNbrLights == -1)
        maxNbrLights = Light::GetMaxNbrLights();

    int curNbrLights = 0;

    std::multimap<float, Light*>::iterator lightIt;
    for(lightIt = lightList.begin() ; lightIt != lightList.end()
    && curNbrLights < maxNbrLights ; ++lightIt)
    {
        Light* curLight = lightIt->second;
        SceneNode* node = curLight->GetParentNode();

        if(m_shadowCastingOption == AllShadows
           || (m_shadowCastingOption == DirectionnalShadow && curLight->GetType() == DirectionnalLight)
           || (m_shadowCastingOption == DynamicShadow && curLight->GetType() == OmniLight))
        if(node != nullptr)
        {
            if(curLight->IsCastShadowEnabled())
                curLight->RenderShadowMap(view);

            ++curNbrLights;
        }
    }
}**/


/*void Scene::AddToRenderQueue(SceneNode *curNode)
{
    if(curNode != nullptr)
    {
        SceneEntityIterator entityIt = curNode->GetEntityIterator();
        while(!entityIt.IsAtTheEnd())
        {
           // if(entityIt.GetElement()->IsRenderable())
                m_renderQueue.push_back(entityIt.GetElement());
            ++entityIt;
        }

        SceneNodeIterator nodeIt = curNode->GetChildIterator();
        while(!nodeIt.IsAtTheEnd())
        {
            AddToRenderQueue(nodeIt.GetElement());
            ++nodeIt;
        }
    }
}*/

SceneNode *Scene::getRootNode()
{
    return &m_rootNode;
}

/**RectEntity* Scene::CreateRectEntity(sf::Vector2f rectSize)
{
    RectEntity *e = new RectEntity(rectSize);
    AddCreatedObject(GenerateObjectID(), e);
    return e;
}

SpriteEntity* Scene::CreateSpriteEntity(sf::Vector2i spriteSize)
{
    return CreateSpriteEntity(sf::IntRect(0,0,spriteSize.x,spriteSize.y));
}

SpriteEntity* Scene::CreateSpriteEntity(sf::IntRect textureRect)
{
    SpriteEntity *e = new SpriteEntity(textureRect);
    AddCreatedObject(GenerateObjectID(), e);
    return e;
}

Light* Scene::CreateLight(LightType type, sf::Vector3f direction, sf::Color color)
{
    Light* light = new Light();
    light->SetType(type);
    light->SetDirection(direction);
    light->SetDiffuseColor(color);
    light->SetShadowMapSize(DEFAULT_SHADOWMAP_SIZE);
    AddCreatedObject(GenerateObjectID(), light);
    return light;
}

Camera* Scene::CreateCamera(sf::Vector2f viewSize)
{
    Camera* camera = new Camera();
    camera->SetSize(viewSize);
    AddCreatedObject(GenerateObjectID(), camera);
    return camera;
}**/


void Scene::addCreatedObject(const ObjectTypeID &id, SceneObject* obj)
{
    auto entityIt = m_createdObjects.find(id);

    if(entityIt != m_createdObjects.end())
    {
        std::ostringstream warn_report;
        warn_report << "Adding scene object of same id as another one (ID="<<id<<")";
        Logger::warning(warn_report);
    }

    m_createdObjects[id] = obj;
}


void Scene::destroyCreatedObject(const ObjectTypeID &id)
{
    auto objIt = m_createdObjects.find(id);

    if(objIt == m_createdObjects.end())
    {
        std::ostringstream error_report;
        error_report << "Cannot destroy scene object (ID="<<id<<")";
        Logger::error(error_report);
    } else {
        if(objIt->second != nullptr)
            delete objIt->second;
        m_createdObjects.erase(objIt);
    }
}

void Scene::destroyAllCreatedObjects()
{
    for(auto it : m_createdObjects)
        if(it.second != nullptr)
            delete it.second;
    m_createdObjects.clear();

    /*while(!m_createdObjects.empty())
    {
        if(m_createdObjects.begin()->second != nullptr)
            delete m_createdObjects.begin()->second;
        m_createdObjects.erase(m_createdObjects.begin());
    }*/
}

/**sf::View Scene::generateView(Camera* cam)
{
    sf::View v;
    if(cam != nullptr)
    {
        v.setSize(cam->GetSize()*cam->GetZoom());
        SceneNode *node = cam->GetParentNode();
        if(node != nullptr)
        {
            sf::Vector3f globalPos = node->GetGlobalPosition();
            v.setCenter(globalPos.x, globalPos.y);
        }
    }
    return v;
}

sf::Vector2f Scene::ConvertMouseToScene(sf::Vector2i mouse)
{
    sf::Vector2f scenePos = sf::Vector2f(mouse);
    if(m_last_target != nullptr && m_currentCamera != nullptr)
    {
        sf::View oldView = m_last_target->getView();
        m_last_target->setView(GenerateView(m_currentCamera));
        scenePos = m_last_target->mapPixelToCoords(mouse);
        m_last_target->setView(oldView);
    }
    return scenePos;
}

void Scene::SetCurrentCamera(Camera *cam)
{
    m_currentCamera = cam;
}

void Scene::SetAmbientLight(sf::Color light)
{
    m_ambientLight = light;
    //m_ambientLight.a = 255;
}

void Scene::SetShadowCasting(ShadowCastingType type)
{
    m_shadowCastingOption = type;
}


void Scene::EnableGammaCorrection()
{
    m_enableSRGB = true;
}

void Scene::DisableGammaCorrection()
{
    m_enableSRGB = false;
}

**/

ObjectTypeID Scene::generateObjectID()
{
    return m_curNewId++;
}


}
