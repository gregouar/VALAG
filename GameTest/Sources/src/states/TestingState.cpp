#include "states/TestingState.h"

#include "Valag/core/StatesManager.h"
#include "Valag/utils/Clock.h"

#include "Valag/gfx/SceneNode.h"
#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"

TestingState::TestingState() :
    m_firstEntering(true),
    m_scene(nullptr)
{
}

TestingState::~TestingState()
{
    if(m_scene != nullptr)
        delete m_scene;
}

void TestingState::init()
{
    m_firstEntering = false;

    m_scene = new vlg::Scene();

    vlg::TextureHandler* textureHandler =  vlg::TextureHandler::instance();

    textureHandler->loadAssetFromFile("../data/sand_color.png");
    textureHandler->loadAssetFromFile("../data/sand_height.png");
    textureHandler->loadAssetFromFile("../data/sand_normal.png");

    textureHandler->loadAssetFromFile("../data/abbey_color.png",vlg::LoadType_InThread);
    textureHandler->loadAssetFromFile("../data/abbey_height.png",vlg::LoadType_InThread);
    textureHandler->loadAssetFromFile("../data/abbey_normal.png",vlg::LoadType_InThread);

    textureHandler->loadAssetFromFile("../data/tree_albedo.png");
    textureHandler->loadAssetFromFile("../data/tree_height.png");
    textureHandler->loadAssetFromFile("../data/tree_normal.png");
    textureHandler->loadAssetFromFile("../data/tree_material.png");

    vlg::SceneNode *abbeyNode =  m_scene->getRootNode()->createChildNode();

    //m_testingSprite.setSize(glm::vec2(150,150));
    //m_testingSprite.setPosition(glm::vec2(100,200));
    m_testingSprites.resize(3);

    auto it = m_testingSprites.begin();
    for(size_t i = 0 ; i < m_testingSprites.size() ; ++i,++it)
    {
        it->setSize(glm::vec2(50,50+i*20));
        it->setPosition(glm::vec2(70+i*80,200+10*i));
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

    if(eventsManager->mouseButtonIsPressed(GLFW_MOUSE_BUTTON_LEFT))
        (++m_testingSprites.begin())->setPosition(eventsManager->mousePosition());


    if(eventsManager->mouseButtonReleased(GLFW_MOUSE_BUTTON_RIGHT))
    {
        m_testingSprites.push_back(vlg::Sprite ());
        (--m_testingSprites.end())->setPosition(eventsManager->mousePosition());
        (--m_testingSprites.end())->setSize(glm::vec2(100,100));
    }

   /** if(event_manager->KeyIsPressed(sf::Keyboard::Left))
        m_camMove.x = -1;
    else if(event_manager->KeyIsPressed(sf::Keyboard::Right))
        m_camMove.x = 1;
    else
        m_camMove.x = 0;

    if(event_manager->KeyIsPressed(sf::Keyboard::Up))
        m_camMove.y = -1;
    else if(event_manager->KeyIsPressed(sf::Keyboard::Down))
        m_camMove.y = 1;
    else
        m_camMove.y = 0;

    if(event_manager->KeyIsPressed(sf::Keyboard::PageUp))
        m_camMove.z = 1;
    else if(event_manager->KeyIsPressed(sf::Keyboard::PageDown))
        m_camMove.z = -1;
    else
        m_camMove.z = 0;


    if(event_manager->KeyIsPressed(sf::Keyboard::Num1))
        m_waterEntity->SetWaveAmplitude(m_waterEntity->GetWaveAmplitude()+0.1);
    if(event_manager->KeyIsPressed(sf::Keyboard::Num2))
        m_waterEntity->SetWaveAmplitude(m_waterEntity->GetWaveAmplitude()-0.1);

    if(event_manager->KeyIsPressed(sf::Keyboard::Num3))
        m_waterEntity->SetWaveSteepness(m_waterEntity->GetWaveSteepness()+0.1);
    if(event_manager->KeyIsPressed(sf::Keyboard::Num4))
        m_waterEntity->SetWaveSteepness(m_waterEntity->GetWaveSteepness()-0.1);

    if(event_manager->KeyIsPressed(sf::Keyboard::Num5))
        m_waterEntity->SetTurbulenceAmplitude(m_waterEntity->GetTurbulenceAmplitude()+0.1);
    if(event_manager->KeyIsPressed(sf::Keyboard::Num6))
        m_waterEntity->SetTurbulenceAmplitude(m_waterEntity->GetTurbulenceAmplitude()-0.1);


    if(event_manager->KeyPressed(sf::Keyboard::E))
        m_mainScene.SetEdgeSmoothing(false);
    if(event_manager->KeyReleased(sf::Keyboard::E))
        m_mainScene.SetEdgeSmoothing(true);

    if(event_manager->KeyPressed(sf::Keyboard::F))
        m_mainScene.SetFoamSimulation(false);
    if(event_manager->KeyReleased(sf::Keyboard::F))
        m_mainScene.SetFoamSimulation(true);

    if(event_manager->KeyPressed(sf::Keyboard::O))
        m_mainScene.SetSSAO(false);
    if(event_manager->KeyReleased(sf::Keyboard::O))
        m_mainScene.SetSSAO(true);

    if(event_manager->KeyPressed(sf::Keyboard::B))
        m_mainScene.SetBloom(false);
    if(event_manager->KeyReleased(sf::Keyboard::B))
        m_mainScene.SetBloom(true);


    if(event_manager->KeyPressed(sf::Keyboard::S))
        m_mainScene.SetShadowCasting(NoShadow);
    if(event_manager->KeyReleased(sf::Keyboard::S))
        m_mainScene.SetShadowCasting(AllShadows);

    if(event_manager->KeyPressed(sf::Keyboard::R))
        m_mainScene.SetSSR(false);
    if(event_manager->KeyReleased(sf::Keyboard::R))
        m_mainScene.SetSSR(true);


    if(event_manager->KeyPressed(sf::Keyboard::G))
        m_mainScene.DisableGammaCorrection();
    if(event_manager->KeyReleased(sf::Keyboard::G))
        m_mainScene.EnableGammaCorrection();


    if(event_manager->MouseButtonIsPressed(sf::Mouse::Left))
    {
        sf::Vector2i p(event_manager->MousePosition());
        m_chene_node->SetPosition(m_mainScene.ConvertMouseToScene(p));

      //  std::cout<<m_mainScene.ConvertMouseToScene(p).x<<" "
               // <<m_mainScene.ConvertMouseToScene(p).y<<std::endl;
    }

    if(event_manager->MouseButtonIsPressed(sf::Mouse::Right))
    {
        sf::Vector2i p(event_manager->MousePosition());
        //m_chene_node->SetPosition(m_mainScene.ConvertMouseToScene(p));

        m_torusPos = sf::Vector2f(m_mainScene.ConvertMouseToScene(p)) - sf::Vector2f(-384+256+2048-128,-768+1024+256);


        //m_cameraNode->SetPosition(m_mainScene.ConvertMouseToScene(p)+sf::Vector2f(3000,3000));
    }


    sf::Vector2f p = m_mainScene.ConvertMouseToScene(event_manager->MousePosition());
    m_lightNode->SetPosition(p.x+80,p.y+80,150);**/

}

void TestingState::update(const vlg::Time &elapsedTime)
{
    m_totalTime += elapsedTime;
    m_nbrFps++;

    if(m_totalTime.count() > 1)
    {
        m_totalTime -= std::chrono::seconds(1);
        std::cout<<"FPS : "<<m_nbrFps<<std::endl;
        m_nbrFps = 0;
    }
}

void TestingState::draw(vlg::DefaultRenderer *renderer  /**sf::RenderTarget* renderer**/)
{
    for(auto &sprite : m_testingSprites)
        renderer->draw(&sprite);
    //renderer->draw(&m_testingSprite);
    //renderer->draw(&m_testingSprite2);
}



