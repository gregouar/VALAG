#include "states/TestingState.h"

#include "Valag/core/StatesManager.h"
#include "Valag/utils/Clock.h"

#include "Valag/scene/SceneNode.h"
#include "Valag/assets/AssetHandler.h"
#include "Valag/assets/TextureAsset.h"
#include "Valag/assets/MaterialAsset.h"
//#include "Valag/assets/MeshAsset.h"
#include "Valag/assets/MeshesHandler.h"


#include "Valag/renderers/DefaultRenderer.h"
#include "Valag/renderers/InstancingRenderer.h"
#include "Valag/renderers/SceneRenderer.h"

#include "Valag/Types.h"

#include <glm/gtc/random.hpp>

TestingState::TestingState() :
    m_firstEntering(true),
    m_scene(nullptr),
    m_testingSpritesBatch(true)
{
    m_nbrFps = 0;
}

TestingState::~TestingState()
{
    this->leaving();
}

void TestingState::init()
{
    m_firstEntering = false;

    m_camVelocity = glm::vec2(0,0);

    m_scene = new vlg::Scene();

    vlg::AssetLoadType loadType = vlg::LoadType_InThread;

    vlg::TexturesHandler* textureHandler =  vlg::TexturesHandler::instance();

    vlg::AssetTypeId tex[10];
    tex[0] = textureHandler->loadAssetFromFile("../data/sand_color.png",loadType)->getId();
    tex[1] = textureHandler->loadAssetFromFile("../data/sand_height.png",loadType)->getId();
    tex[2] = textureHandler->loadAssetFromFile("../data/sand_normal.png",loadType)->getId();

    tex[3] = textureHandler->loadAssetFromFile("../data/abbey_albedo.png",loadType)->getId();
    tex[4] = textureHandler->loadAssetFromFile("../data/abbey_height.png",loadType)->getId();
    tex[5] = textureHandler->loadAssetFromFile("../data/abbey_normal.png",loadType)->getId();

    tex[6] = textureHandler->loadAssetFromFile("../data/tree_albedo.png",loadType)->getId();
    tex[7] = textureHandler->loadAssetFromFile("../data/tree_height.png",loadType)->getId();
    tex[8] = textureHandler->loadAssetFromFile("../data/tree_height.png",loadType)->getId();
    tex[9] = textureHandler->loadAssetFromFile("../data/tree_rmt.png",loadType)->getId();


    m_testingSprites.resize(2);

    auto it = m_testingSprites.begin();
    for(size_t i = 0 ; i < m_testingSprites.size() ; ++i,++it)
    {
        it->setSize(glm::vec2(glm::linearRand(25,100), glm::linearRand(25,100)));
        it->setPosition(glm::vec2(glm::linearRand(0, 1024),glm::linearRand(0, 768)));
        it->setColor(glm::vec4(glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(.5f,1.0f)));

        it->setTexture(tex[i%10]);
    }


    m_testingSpritesInBatch.resize(100);

    it = m_testingSpritesInBatch.begin();
    for(size_t i = 0 ; i < m_testingSpritesInBatch.size() ; ++i,++it)
    {
        it->setSize(glm::vec2(glm::linearRand(25,100), glm::linearRand(25,100)));
        it->setPosition(glm::vec3(glm::linearRand(0, 1024),glm::linearRand(0, 768),i));
        it->setColor(glm::vec4(glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(.5f,1.0f)));

        it->setTexture(tex[i%10]);

        m_testingSpritesBatch.addSprite(&(*it));
    }

    /// SCENE

    m_scene->setAmbientLight({96/255.0,127/255.0,196/255.0,128.0/255.0});
   // m_scene->setAmbientLight({96/255.0,127/255.0,255/255.0,128.0/255.0});
    //m_scene->setAmbientLight({16/255.0,32/255.0,255/255.0,96.0/255.0});

    //m_scene->setEnvironmentMap(vlg::TexturesHandler::instance()->loadAssetFromFile("../data/HDRenv.hdr",loadType));
    m_scene->setEnvironmentMap(vlg::TexturesHandler::instance()->loadAssetFromFile("../data/panorama.jpg",loadType));

    vlg::MaterialAsset *abbeyMaterial = vlg::MaterialsHandler::instance()->loadAssetFromFile("../data/abbeyXML.txt",loadType);
    vlg::MaterialAsset *treeMaterial = vlg::MaterialsHandler::instance()->loadAssetFromFile("../data/treeXML.txt",loadType);

    m_treeModel = new vlg::IsoSpriteModel();
    m_treeModel->setMaterial(treeMaterial);
    m_treeModel->setSize({512.0,512.0});
    m_treeModel->setTextureRect({0,0},{1,1});
    m_treeModel->setTextureCenter({256,526});

    m_abbeyModel = new vlg::IsoSpriteModel();
    m_abbeyModel->setMaterial(abbeyMaterial);
    m_abbeyModel->setSize({1920,1080});
    m_abbeyModel->setTextureRect({0,0},{1,1});
    m_abbeyModel->setTextureCenter({1920/2,1080/2});


    m_abbeyNode =  m_scene->getRootNode()->createChildNode({0,100,-1});
    m_abbeyNode->attachObject(m_scene->createIsoSpriteEntity(m_abbeyModel));

    //m_treeEntity.setSpriteModel(&m_treeModel);
    //m_abbeyEntity.setSpriteModel(&m_abbeyModel);

    m_treeNode  =  m_scene->getRootNode()->createChildNode({900,400,-90});
    m_treeEntity = m_scene->createIsoSpriteEntity(m_treeModel);
    m_treeEntity->setShadowCasting(vlg::ShadowCasting_OnlyDirectional);
    //m_treeEntity->setColor({0.0,1.0,0.0,0.6});
    m_treeNode->attachObject(m_treeEntity);

    for(size_t x = 0 ; x < 0 ; x++)
    for(size_t y = 0 ; y < 0 ; y++)
    {
        //m_forestEntities.push_back(vlg::IsoSpriteEntity());
        //m_forestEntities.back().setSpriteModel(&m_treeModel);
        m_scene ->getRootNode()
                ->createChildNode({x*50,y*50,-90})
                ->attachObject(m_scene->createIsoSpriteEntity(m_treeModel));
    }

    m_camera = m_scene->createCamera();
    //m_camera->setViewport({.2,.3},{.5,.4});
    m_cameraNode = m_scene->getRootNode()->createChildNode(2000,2000,1500);
    m_cameraNode->attachObject(m_camera);
    //m_scene->setCurrentCamera(m_camera);
    m_scene->setViewAngle(glm::pi<float>()/4.0f, //45
                          glm::pi<float>()/6.0f); //30

    m_quackMesh = vlg::MeshesHandler::instance()->loadAssetFromFile("../data/quackXML.txt",loadType);

    //m_quackEntities.push_back(vlg::MeshEntity());
    //m_quackEntities.back().setMesh(m_quackMesh);
    m_quackNode = m_scene->getRootNode()->createChildNode(250,450);
    m_quackNode->attachObject(m_scene->createMeshEntity(m_quackMesh));
    m_quackNode->scale(5.0f);

    vlg::MaterialAsset *groundSand = vlg::MaterialsHandler::instance()->loadAssetFromFile("../data/wetSandXML.txt",loadType);
    vlg::MeshAsset     *groundMesh = vlg::MeshesHandler::makeQuad({-512,-512},{2048,2048},groundSand,{0,0},{4.0,4.0});
    //m_groundSand.setMesh(groundMesh);
    m_scene->getRootNode()->createChildNode({0,0,-2})->attachObject(m_scene->createMeshEntity(groundMesh));

    vlg::LightEntity* sunLight = m_scene->createLightEntity(vlg::LightType_Directional);
    m_scene->getRootNode()->attachObject(sunLight);

    ///Day
    sunLight->setDiffuseColor({1.0,1.0,1.0,1.0});
    sunLight->setIntensity(15.0);

    ///Night
    //m_sunLight.setDiffuseColor({0.7,0.7,1.0,1.0});
    //m_sunLight.setIntensity(0.2);

    //sunLight->setType(vlg::LightType_Directional);
    //m_sunLight.setDirection({-1.0,0.0,-1.0});
    sunLight->setDirection({.2 ,-1.0,-1.0});
    sunLight->enableShadowCasting();

    sunLight = m_scene->createLightEntity(vlg::LightType_Directional);
    m_scene->getRootNode()->attachObject(sunLight);
    sunLight->setDirection({-.2 ,-1.0,-1.0});
    sunLight->enableShadowCasting();


    vlg::LightEntity *cursorLight = m_scene->createLightEntity();
    m_cursorLightNode = m_scene->getRootNode()->createChildNode(0,0,60);
    m_cursorLightNode->attachObject(cursorLight);
    cursorLight->setDiffuseColor({1.0,1.0,1.0,1.0});
    cursorLight->setIntensity(10.0);
    cursorLight->setRadius(400.0);
    cursorLight->setType(vlg::LightType_Omni);

    for(size_t i = 3 ; i < 3 ; ++i)
    {
        vlg::LightEntity *secLight = m_scene->createLightEntity();
        secLight->setDiffuseColor({glm::linearRand(0,100)/100.0,
                                    glm::linearRand(0,100)/100.0,
                                    glm::linearRand(0,100)/100.0});

        secLight->setIntensity(glm::linearRand(2.0,20.0));
        secLight->setRadius(glm::linearRand(100.0,600.0));
        m_cursorLightNode->createChildNode({glm::linearRand(-600,600),
                                            glm::linearRand(-600,600),
                                            glm::linearRand(-50,300)})->attachObject(secLight);
    }


  /*  for(size_t i = 0 ; i < 8 ; ++i)
    {
        //vec3(.4,0,.8)
        glm::vec3 v;
        v.x = glm::linearRand(-100,100)/100.0;
        v.y = glm::linearRand(-100,100)/100.0;
        v.z = glm::linearRand(0,100)/100.0;
        v = glm::normalize(v);
        v *= glm::linearRand(10,100)/100.0;

        std::ostringstream buf;
        buf<<"vec3("<<v.x<<","<<v.y<<","<<v.z<<"),";
        Logger::write(buf);

        v.x *= -1;
        v.y *= -1;

        std::ostringstream buf2;
        buf2<<"vec3("<<v.x<<","<<v.y<<","<<v.z<<"),";
        Logger::write(buf2);
    }*/
}

void TestingState::entered()
{
    m_totalTime = vlg::TimeZero();

    if(m_firstEntering)
        this->init();
}

void TestingState::leaving()
{
    if(m_scene != nullptr)
        delete m_scene;
    m_scene = nullptr;

    //I should have some kind of spriteModel manager (maybe I should put them as assets ?)

    if(m_treeModel != nullptr)
        delete m_treeModel;
    m_treeModel = nullptr;

    if(m_abbeyModel != nullptr)
        delete m_abbeyModel;
    m_abbeyModel = nullptr;
}

void TestingState::revealed()
{

}

void TestingState::obscuring()
{

}

void TestingState::handleEvents(const EventsManager *eventsManager)
{
    if(eventsManager->keyReleased(GLFW_KEY_ESCAPE))
        m_manager->stop();

    if(eventsManager->isAskingToClose())
        m_manager->stop();

    if(m_scene == nullptr)
        return;

    glm::vec2 worldMousePos = m_scene->convertScreenToWorldCoord(eventsManager->centeredMousePosition(), m_camera);

    //std::cout<<worldMousePos.x<<" "<<worldMousePos.y<<std::endl;
    m_camVelocity = {0,0};

    m_cursorLightNode->setPosition(worldMousePos);

    if(eventsManager->keyPressed(GLFW_KEY_A))
        m_treeEntity->setRotation(m_treeEntity->getRotation()+0.1f);
    if(eventsManager->keyPressed(GLFW_KEY_Z))
        m_treeEntity->setRotation(m_treeEntity->getRotation()-0.1f);


    if(eventsManager->keyPressed(GLFW_KEY_Q))
        m_abbeyNode->move(0,0,100);
    if(eventsManager->keyPressed(GLFW_KEY_S))
        m_abbeyNode->move(0,0,-100);

    if(eventsManager->keyIsPressed(GLFW_KEY_DOWN))
        m_camVelocity.y = 200.0;
        //m_treeNode->move(0,0,-5);
    if(eventsManager->keyIsPressed(GLFW_KEY_UP))
        m_camVelocity.y = -200.0;
    if(eventsManager->keyIsPressed(GLFW_KEY_LEFT))
        m_camVelocity.x = -200.0;
    if(eventsManager->keyIsPressed(GLFW_KEY_RIGHT))
        m_camVelocity.x = 200.0;
        //m_treeNode->move(0,0,5);


    if(eventsManager->keyPressed(GLFW_KEY_SPACE))
    {
        glm::vec3 oldPos = m_quackNode->getPosition();
        glm::vec3 oldRot = m_quackNode->getEulerRotation();

        //m_quackEntities.push_back(vlg::MeshEntity());
        //m_quackEntities.back().setMesh(m_quackMesh);
        m_quackNode = m_scene->getRootNode()->createChildNode();
        m_quackNode->attachObject(m_scene->createMeshEntity(m_quackMesh));
        m_quackNode->scale(5.0f);

        m_quackNode->setPosition(oldPos);
        m_quackNode->setRotation(oldRot);
    }

    if(eventsManager->mouseButtonIsPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        /*(++m_testingSprites.begin())->setPosition(eventsManager->mousePosition());
        (++m_testingSprites.begin())->setSize({256,256});*/
        m_treeNode->setPosition(worldMousePos);
    }

    if(eventsManager->mouseButtonIsPressed(GLFW_MOUSE_BUTTON_RIGHT))
    {
        //m_abbeyNode->setPosition(eventsManager->mousePosition());
        m_quackNode->setPosition(worldMousePos);
    }

    /*if(eventsManager->mouseButtonReleased(GLFW_MOUSE_BUTTON_RIGHT))
        for(size_t j = 0 ; j < 1000 ; ++j)
    {
        m_testingSprites.resize(m_testingSprites.size() + 1);
        (--m_testingSprites.end())->setPosition(glm::vec3(eventsManager->mousePosition()+glm::vec2(j,j%100), j));
        (--m_testingSprites.end())->setSize(glm::vec2(100,100));
        (--m_testingSprites.end())->setTexture(vlg::TexturesHandler::instance()->loadAssetFromFile("../data/tree_normal.png",loadType)->getId());
    }*/

}

void TestingState::update(const vlg::Time &elapsedTime)
{
    m_totalTime += elapsedTime;
    m_nbrFps++;

    glm::vec2 camMove = m_scene->convertScreenToWorldCoord(m_camVelocity);
    camMove.x *= elapsedTime.count();
    camMove.y *= elapsedTime.count();
    m_cameraNode->move(camMove);

    m_scene->update(elapsedTime);

    if(m_totalTime.count() > 1)
    {
        m_totalTime -= std::chrono::seconds(1);
        std::cout<<"FPS : "<<m_nbrFps<<std::endl;
        m_nbrFps = 0;
    }

    //m_quackEntities.back().setRmt({5.0*m_totalTime.count(),1.0,1.0});

    m_quackNode->rotate(elapsedTime.count(), {0,0,1});
    m_cursorLightNode ->rotate(elapsedTime.count(), {0,0,1});

   // m_testingSprites.front().setColor(glm::vec4(1,m_totalTime.count(),m_totalTime.count(),1));
}

void TestingState::draw(vlg::RenderWindow *renderWindow)
{
    if(renderWindow->getRenderer(vlg::Renderer_Scene) != nullptr)
    {
        vlg::SceneRenderer *renderer = dynamic_cast<vlg::SceneRenderer*>(renderWindow->getRenderer(vlg::Renderer_Scene));
        //renderer->draw(&m_abbeyEntity);
        //renderer->draw(&m_treeEntity);
        m_scene->render(renderer, m_camera);
    }

    //vlg::DefaultRenderer *renderer = dynamic_cast<vlg::DefaultRenderer*>(renderWindow->getRenderer(vlg::Renderer_Default));
    if(renderWindow->getRenderer(vlg::Renderer_Instancing) != nullptr)
    {
        vlg::InstancingRenderer *renderer = dynamic_cast<vlg::InstancingRenderer*>(renderWindow->getRenderer(vlg::Renderer_Instancing));

        for(auto &sprite : m_testingSprites)
            renderer->draw(&sprite);

        renderer->draw(&m_testingSpritesBatch);
    }


}



