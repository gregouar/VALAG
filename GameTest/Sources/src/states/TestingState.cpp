#include "states/TestingState.h"

#include "Valag/core/StatesManager.h"
#include "Valag/utils/Clock.h"

#include "Valag/gfx/SceneNode.h"
#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"

#include <glm/gtc/random.hpp>

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

    vlg::AssetTypeID tex3 = textureHandler->loadAssetFromFile("../data/sand_color.png")->getID();
    textureHandler->loadAssetFromFile("../data/sand_height.png",vlg::LoadType_InThread);
    textureHandler->loadAssetFromFile("../data/sand_normal.png",vlg::LoadType_InThread);

    vlg::AssetTypeID tex4 = textureHandler->loadAssetFromFile("../data/abbey_color.png",vlg::LoadType_InThread)->getID();
    textureHandler->loadAssetFromFile("../data/abbey_height.png",vlg::LoadType_InThread);
    textureHandler->loadAssetFromFile("../data/abbey_normal.png",vlg::LoadType_InThread);

    vlg::AssetTypeID tex = textureHandler->loadAssetFromFile("../data/tree_albedo.png",vlg::LoadType_InThread)->getID();
    vlg::AssetTypeID tex2 = textureHandler->loadAssetFromFile("../data/tree_height.png",vlg::LoadType_InThread)->getID();
    textureHandler->loadAssetFromFile("../data/tree_normal.png");
    textureHandler->loadAssetFromFile("../data/tree_material.png");

    vlg::SceneNode *abbeyNode =  m_scene->getRootNode()->createChildNode();

    //m_testingSprite.setSize(glm::vec2(150,150));
    //m_testingSprite.setPosition(glm::vec2(100,200));
    m_testingSprites.resize(2);

    auto it = m_testingSprites.begin();
    for(size_t i = 0 ; i < m_testingSprites.size() ; ++i,++it)
    {
        /*it->setSize(glm::vec2(glm::linearRand(25,100), glm::linearRand(25,100)));
        it->setPosition(glm::vec2(glm::linearRand(0, 1024),glm::linearRand(0, 768)));
        it->setColor(glm::vec4(glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(.5f,1.0f)));
        if(i % 2 == 0)
        it->setTexture(tex);*/

        it->setSize(glm::vec2(glm::linearRand(25,100), glm::linearRand(25,100)));
        it->setPosition(glm::vec2(glm::linearRand(0, 1024),glm::linearRand(0, 768)));
        it->setColor(glm::vec4(glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(.5f,1.0f)));

        if(i % 3 == 0)
            it->setTexture(tex2);
        if(i % 3 == 1)
            it->setTexture(tex3);
        if(i % 3 == 2)
            it->setTexture(tex4);
    }


    m_testingSpritesInBatch.resize(10);

    it = m_testingSpritesInBatch.begin();
    for(size_t i = 0 ; i < m_testingSpritesInBatch.size() ; ++i,++it)
    {
        it->setSize(glm::vec2(glm::linearRand(25,100), glm::linearRand(25,100)));
        it->setPosition(glm::vec2(glm::linearRand(0, 1024),glm::linearRand(0, 768)));
        it->setColor(glm::vec4(glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(.5f,1.0f)));

        if(i % 3 == 0)
            it->setTexture(tex2);
        if(i % 3 == 1)
            it->setTexture(tex3);
        if(i % 3 == 2)
            it->setTexture(tex4);

        m_testingSpritesBatch.addSprite(&(*it));
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
        (++m_testingSpritesInBatch.begin())->setPosition(eventsManager->mousePosition());

    if(eventsManager->mouseButtonReleased(GLFW_MOUSE_BUTTON_RIGHT))
        for(size_t j = 0 ; j < 2000 ; ++j)
    {
        m_testingSpritesInBatch.resize(m_testingSpritesInBatch.size() + 1);
      //  m_testingSpritesInBatch.push_back(vlg::Sprite ()); WTF ????
        (--m_testingSpritesInBatch.end())->setPosition(eventsManager->mousePosition());
        (--m_testingSpritesInBatch.end())->setSize(glm::vec2(100/*+j*/,100));
        (--m_testingSpritesInBatch.end())->setTexture(vlg::TextureHandler::instance()->loadAssetFromFile("../data/tree_normal.png")->getID());
                                               //m_testingSprites.front().getTexture());
        m_testingSpritesBatch.addSprite(&(*(--m_testingSpritesInBatch.end())));
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

    m_testingSprites.front().setColor(glm::vec4(1,m_totalTime.count(),m_totalTime.count(),1));
}

void TestingState::draw(vlg::DefaultRenderer *renderer  /**sf::RenderTarget* renderer**/)
{
    for(auto &sprite : m_testingSprites)
        renderer->draw(&sprite);

    renderer->draw(&m_testingSpritesBatch);
}



