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
    if(m_scene != nullptr)
        delete m_scene;
}

void TestingState::init()
{
    m_firstEntering = false;

    m_camVelocity = glm::vec2(0,0);

    m_scene = new vlg::Scene();

    vlg::TexturesHandler* textureHandler =  vlg::TexturesHandler::instance();

    vlg::AssetTypeId tex[10];
    tex[0] = textureHandler->loadAssetFromFile("../data/sand_color.png",vlg::LoadType_InThread)->getId();
    tex[1] = textureHandler->loadAssetFromFile("../data/sand_height.png",vlg::LoadType_InThread)->getId();
    tex[2] = textureHandler->loadAssetFromFile("../data/sand_normal.png",vlg::LoadType_InThread)->getId();

    tex[3] = textureHandler->loadAssetFromFile("../data/abbey_albedo.png",vlg::LoadType_InThread)->getId();
    tex[4] = textureHandler->loadAssetFromFile("../data/abbey_height.png",vlg::LoadType_InThread)->getId();
    tex[5] = textureHandler->loadAssetFromFile("../data/abbey_normal.png",vlg::LoadType_InThread)->getId();

    tex[6] = textureHandler->loadAssetFromFile("../data/tree_albedo.png",vlg::LoadType_InThread)->getId();
    tex[7] = textureHandler->loadAssetFromFile("../data/tree_height.png",vlg::LoadType_InThread)->getId();
    tex[8] = textureHandler->loadAssetFromFile("../data/tree_height.png",vlg::LoadType_InThread)->getId();
    tex[9] = textureHandler->loadAssetFromFile("../data/tree_rmt.png",vlg::LoadType_InThread)->getId();


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

    //m_scene->setAmbientLight({0.4,0.4,1.0,0.2});
    m_scene->setAmbientLight({96/255.0,127/255.0,255/255.0,96.0/255.0});
   // m_scene->setAmbientLight({1.0,1.0,1.0,0.5});
    //m_scene->setEnvironmentMap(vlg::TexturesHandler::instance()->loadAssetFromFile("../data/HDRenv.exr",vlg::LoadType_InThread));
    m_scene->setEnvironmentMap(vlg::TexturesHandler::instance()->loadAssetFromFile("../data/panorama.jpg",vlg::LoadType_InThread));

    vlg::MaterialAsset *abbeyMaterial = vlg::MaterialsHandler::instance()->loadAssetFromFile("../data/abbeyXML.txt",vlg::LoadType_InThread);
    vlg::MaterialAsset *treeMaterial = vlg::MaterialsHandler::instance()->loadAssetFromFile("../data/treeXML.txt",vlg::LoadType_InThread);

    m_treeModel.setMaterial(treeMaterial);
    m_treeModel.setSize({512.0,512.0});
    m_treeModel.setTextureRect({0,0},{1,1});
    m_treeModel.setTextureCenter({256,526});

    m_abbeyModel.setMaterial(abbeyMaterial);
    m_abbeyModel.setSize({1920,1080});
    m_abbeyModel.setTextureRect({0,0},{1,1});
    m_abbeyModel.setTextureCenter({1920/2,1080/2});

    m_treeEntity.setSpriteModel(&m_treeModel);
    m_abbeyEntity.setSpriteModel(&m_abbeyModel);

    m_treeNode  =  m_scene->getRootNode()->createChildNode({0,0,-90});
    m_abbeyNode =  m_scene->getRootNode()->createChildNode({0,100,-1});

    m_treeNode->attachObject(&m_treeEntity);
    m_abbeyNode->attachObject(&m_abbeyEntity);

    //m_treeEntity.setColor({0.0,1.0,0.0,0.5});
    //m_treeEntity.setRotation(glm::pi<float>()/6.0f);

    m_camera = m_scene->createCamera();
    m_cameraNode = m_scene->getRootNode()->createChildNode(1500,1500,1500);
    m_cameraNode->attachObject(m_camera);
    m_scene->setCurrentCamera(m_camera);
    m_scene->setViewAngle(glm::pi<float>()/4.0f, //45
                          glm::pi<float>()/6.0f); //30

    m_quackMesh = vlg::MeshesHandler::instance()->loadAssetFromFile("../data/quackXML.txt",vlg::LoadType_InThread);

    m_quackEntities.push_back(vlg::MeshEntity());
    m_quackEntities.back().setMesh(m_quackMesh);
    m_quackNode = m_scene->getRootNode()->createChildNode();
    m_quackNode->attachObject(&m_quackEntities.back());
    m_quackNode->scale(5.0f);


    vlg::MaterialAsset *groundSand = vlg::MaterialsHandler::instance()->loadAssetFromFile("../data/wetSandXML.txt",vlg::LoadType_InThread);
    vlg::MeshAsset     *groundMesh = vlg::MeshesHandler::makeQuad({-512,-512},{2048,2048},groundSand,{0,0},{4.0,4.0});
    m_groundSand.setMesh(groundMesh);
    m_scene->getRootNode()->createChildNode({0,0,-2})->attachObject(&m_groundSand);

    m_scene->getRootNode()->attachObject(&m_sunLight);
    m_sunLight.setDiffuseColor({1.0,1.0,1.0,1.0});
    m_sunLight.setIntensity(15.0);
    m_sunLight.setType(vlg::LightType_Directionnal);
    //m_sunLight.setDirection({-1.0,0.0,-1.0});
    m_sunLight.setDirection({.2 ,-1.0,-1.0});

    m_cursorLightNode = m_scene->getRootNode()->createChildNode(0,0,60);
    m_cursorLightNode->attachObject(&m_cursorLight);
    m_cursorLight.setDiffuseColor({1.0,1.0,1.0,1.0});
    m_cursorLight.setIntensity(10.0);
    m_cursorLight.setRadius(400.0);
    m_cursorLight.setType(vlg::LightType_Omni);

    for(size_t i = 0 ; i < 0 ; ++i)
    {
        m_secLights.push_back(vlg::Light());
        m_secLights.back().setDiffuseColor({glm::linearRand(0,100)/100.0,
                                            glm::linearRand(0,100)/100.0,
                                            glm::linearRand(0,100)/100.0});

        m_secLights.back().setIntensity(glm::linearRand(2.0,20.0));
        m_secLights.back().setRadius(glm::linearRand(100.0,600.0));
        m_secLights.back().setType(vlg::LightType_Omni);
        m_cursorLightNode->createChildNode({glm::linearRand(-600,600),
                                            glm::linearRand(-600,600),
                                            glm::linearRand(-50,300)})->attachObject(&m_secLights.back());
    }
}

void TestingState::entered()
{
    m_totalTime = vlg::TimeZero();

    if(m_firstEntering)
        this->init();
}

void TestingState::leaving()
{
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


    glm::vec2 worldMousePos = m_scene->convertScreenToWorldCoord(eventsManager->centeredMousePosition(), m_camera);

    //std::cout<<worldMousePos.x<<" "<<worldMousePos.y<<std::endl;
    m_camVelocity = {0,0};

    m_cursorLightNode->setPosition(worldMousePos);

    if(eventsManager->keyPressed(GLFW_KEY_A))
        m_treeEntity.setRotation(m_treeEntity.getRotation()+0.1f);
    if(eventsManager->keyPressed(GLFW_KEY_Z))
        m_treeEntity.setRotation(m_treeEntity.getRotation()-0.1f);


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

        m_quackEntities.push_back(vlg::MeshEntity());
        m_quackEntities.back().setMesh(m_quackMesh);
        m_quackNode = m_scene->getRootNode()->createChildNode();
        m_quackNode->attachObject(&m_quackEntities.back());
        m_quackNode->scale(5.0f);

        m_quackNode->setPosition(oldPos);
        m_quackNode->setRotation(oldRot);
    }

    if(eventsManager->mouseButtonIsPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        (++m_testingSprites.begin())->setPosition(eventsManager->mousePosition());
        (++m_testingSprites.begin())->setSize({256,256});
        m_treeNode->setPosition(worldMousePos);
    }

    if(eventsManager->mouseButtonIsPressed(GLFW_MOUSE_BUTTON_RIGHT))
    {
        //m_abbeyNode->setPosition(eventsManager->mousePosition());
        m_quackNode->setPosition(worldMousePos);
    }

    if(eventsManager->mouseButtonReleased(GLFW_MOUSE_BUTTON_RIGHT))
        for(size_t j = 0 ; j < 1000 ; ++j)
    {
        m_testingSprites.resize(m_testingSprites.size() + 1);
        (--m_testingSprites.end())->setPosition(glm::vec3(eventsManager->mousePosition()+glm::vec2(j,j%100), j));
        (--m_testingSprites.end())->setSize(glm::vec2(100,100));
        (--m_testingSprites.end())->setTexture(vlg::TexturesHandler::instance()->loadAssetFromFile("../data/tree_normal.png",vlg::LoadType_InThread)->getId());
        //m_testingSpritesBatch.addSprite(&(*(--m_testingSprites.end())));
    }

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
        m_scene->render(renderer);
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



