#include "states/TestingState.h"

#include "Valag/core/StatesManager.h"
#include "Valag/utils/Clock.h"

#include "Valag/gfx/SceneNode.h"
#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"
#include "Valag/gfx/MaterialAsset.h"
#include "Valag/renderers/DefaultRenderer.h"

#include "Valag/Types.h"

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

    vlg::TexturesHandler* textureHandler =  vlg::TexturesHandler::instance();

    vlg::AssetTypeID tex[10];
    tex[0] = textureHandler->loadAssetFromFile("../data/sand_color.png",vlg::LoadType_InThread)->getID();
    tex[1] = textureHandler->loadAssetFromFile("../data/sand_height.png",vlg::LoadType_InThread)->getID();
    tex[2] = textureHandler->loadAssetFromFile("../data/sand_normal.png",vlg::LoadType_InThread)->getID();

    tex[3] = textureHandler->loadAssetFromFile("../data/abbey_albedo.png"/*,vlg::LoadType_InThread*/)->getID();
    tex[4] = textureHandler->loadAssetFromFile("../data/abbey_height.png"/*,vlg::LoadType_InThread*/)->getID();
    tex[5] = textureHandler->loadAssetFromFile("../data/abbey_normal.png"/*,vlg::LoadType_InThread*/)->getID();

    tex[6] = textureHandler->loadAssetFromFile("../data/tree_albedo.png",vlg::LoadType_InThread)->getID();
    tex[7] = textureHandler->loadAssetFromFile("../data/tree_height.png",vlg::LoadType_InThread)->getID();
    tex[8] = textureHandler->loadAssetFromFile("../data/tree_height.png",vlg::LoadType_InThread)->getID();
    tex[9] = textureHandler->loadAssetFromFile("../data/tree_rmt.png",vlg::LoadType_InThread)->getID();

    vlg::SceneNode *abbeyNode =  m_scene->getRootNode()->createChildNode();

    //m_testingSprite.setSize(glm::vec2(150,150));
    //m_testingSprite.setPosition(glm::vec2(100,200));
    m_testingSprites.resize(5);

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

       /* if(i % 3 == 0)
            it->setTexture(tex[0]);
        if(i % 3 == 1)
            it->setTexture(tex[2]);
        if(i % 3 == 2)
            it->setTexture(tex[6]);*/
        it->setTexture(tex[i%10]);
    }


    m_testingSpritesInBatch.resize(10);

    it = m_testingSpritesInBatch.begin();
    for(size_t i = 0 ; i < m_testingSpritesInBatch.size() ; ++i,++it)
    {
        it->setSize(glm::vec2(glm::linearRand(25,100), glm::linearRand(25,100)));
        it->setPosition(glm::vec3(glm::linearRand(0, 1024),glm::linearRand(0, 768),i));
        it->setColor(glm::vec4(glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(0.0f,1.0f),glm::linearRand(.5f,1.0f)));

        /*if(i < 5000)
            it->setTexture(tex[0]);
        else
            it->setTexture(tex[1]);*/
        it->setTexture(tex[i%10]);

        m_testingSpritesBatch.addSprite(&(*it));
    }

    vlg::AssetHandler<vlg::MaterialAsset>::instance()->loadAssetFromFile("../data/abbeyXML.txt"/*,vlg::LoadType_InThread*/);


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
        for(size_t j = 0 ; j < 1000 ; ++j)
    {
        m_testingSpritesInBatch.resize(m_testingSpritesInBatch.size() + 1);
        (--m_testingSpritesInBatch.end())->setPosition(eventsManager->mousePosition()+glm::vec2(j,j%100));
        (--m_testingSpritesInBatch.end())->setSize(glm::vec2(100,100));
        (--m_testingSpritesInBatch.end())->setTexture(vlg::TexturesHandler::instance()->loadAssetFromFile("../data/tree_normal.png",vlg::LoadType_InThread)->getID());
        m_testingSpritesBatch.addSprite(&(*(--m_testingSpritesInBatch.end())));

       /* m_testingSprites.resize(m_testingSprites.size() + 1);
        (--m_testingSprites.end())->setPosition(eventsManager->mousePosition()+glm::vec2(j,0));
        (--m_testingSprites.end())->setSize(glm::vec2(100,100));
        (--m_testingSprites.end())->setTexture(vlg::TextureHandler::instance()->loadAssetFromFile("../data/tree_normal.png",vlg::LoadType_InThread)->getID());*/
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

void TestingState::draw(vlg::RenderWindow *renderWindow)
{
    vlg::DefaultRenderer *defaultRenderer = dynamic_cast<vlg::DefaultRenderer*>(renderWindow->getRenderer(vlg::Renderer_Default));
    //vlg::DefaultRenderer *defaultRenderer = (vlg::DefaultRenderer*)renderWindow->getRenderer(vlg::Renderer_Default);

    for(auto &sprite : m_testingSprites)
        defaultRenderer->draw(&sprite);

    defaultRenderer->draw(&m_testingSpritesBatch);

}



