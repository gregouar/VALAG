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

    vlg::AssetTypeID tex = textureHandler->loadAssetFromFile("../data/tree_albedo.png",vlg::LoadType_InThread)->getID();
    textureHandler->loadAssetFromFile("../data/tree_height.png");
    textureHandler->loadAssetFromFile("../data/tree_normal.png");
    textureHandler->loadAssetFromFile("../data/tree_material.png");

    vlg::SceneNode *abbeyNode =  m_scene->getRootNode()->createChildNode();

    //m_testingSprite.setSize(glm::vec2(150,150));
    //m_testingSprite.setPosition(glm::vec2(100,200));
    m_testingSprites.resize(5);

    auto it = m_testingSprites.begin();
    for(size_t i = 0 ; i < m_testingSprites.size() ; ++i,++it)
    {
        it->setSize(glm::vec2(50,50+i*20));
        it->setPosition(glm::vec2(70+i*80,200+10*i));
        if(i % 2 == 0)
        it->setTexture(tex);
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
        //for(size_t j = 0 ; j < 200 ; ++j)
    {
        m_testingSprites.resize(m_testingSprites.size() + 1);
      //  m_testingSprites.push_back(vlg::Sprite ()); WTF ????
        (--m_testingSprites.end())->setPosition(eventsManager->mousePosition());
        (--m_testingSprites.end())->setSize(glm::vec2(100/*+j*/,100));
        (--m_testingSprites.end())->setTexture(vlg::TextureHandler::instance()->loadAssetFromFile("../data/tree_normal.png")->getID());
                                               //m_testingSprites.front().getTexture());
    }

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
}



