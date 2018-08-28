#include "Valag/core/VApp.h"

#include "Valag/utils/Parser.h"
#include "Valag/utils/Logger.h"
#include "Valag/utils/Clock.h"
#include "Valag/core/Config.h"

#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"
#include "Valag/vulkanImpl/VMemoryAllocator.h"

#include "Valag/utils/Profiler.h"

namespace vlg
{

const char *VApp::DEFAULT_APP_NAME = "VALAGEngine";
const char *VApp::DEFAULT_CONFIG_FILE = "config.ini";
const char *VApp::DEFAULT_SCREENSHOTPATH = "../screenshots/";
const char *VApp::DEFAULT_SHADERPATH = "../shaders/";

const char *VApp::DEFAULT_WINDOW_FULLSCREEN = "false";
const char *VApp::DEFAULT_WINDOW_WIDTH = "1024";
const char *VApp::DEFAULT_WINDOW_HEIGHT = "768";
const char *VApp::DEFAULT_VSYNC = "false";
const char *VApp::DEFAULT_ANISOTROPIC = "16";


const bool VApp::ENABLE_PROFILER = false;

/** I should replace that by config double/triple buffering at some point **/
const size_t VApp::MAX_FRAMES_IN_FLIGHT = 2;

VApp::VApp(const VAppCreateInfos &infos) :
    m_createInfos(infos),
    m_defaultRenderer(nullptr),
    m_sceneRenderer(nullptr),
    m_sceenshotNbr(1)
{

}

VApp::~VApp()
{
    this->cleanup();
}


void VApp::run(GameState *startingState)
{
    m_running = true;

    if(!this->init())
        throw std::runtime_error("Could not initialize application");

    m_statesManager.attachApp(this);
    m_statesManager.switchState(startingState);

    Logger::write("Starting application");

    this->loop();
}

void VApp::stop()
{
    Logger::write("Stopping application");

    m_running = false;
}

void VApp::printscreen()
{

}


bool VApp::init()
{
    Config::instance()->load(DEFAULT_CONFIG_FILE);

    Logger::write("Initializing application");

    glfwInit();

    if(!this->createWindow())
        throw std::runtime_error("Cannot create window");

    VInstance::instance()->init(m_renderWindow.getSurface()); //Throw error

    if(!this->createDummyAssets())
        throw std::runtime_error("Cannot create dummy assets");

    if(!m_renderWindow.init())
        throw std::runtime_error("Cannot initialize window");

    m_eventsManager.init(m_renderWindow.getWindowPtr());

    if(!this->createDefaultRenderer())
        throw std::runtime_error("Cannot create default renderer");

    if(!this->createSceneRenderer())
        throw std::runtime_error("Cannot create scene renderer");

    return (true);
}

bool VApp::createWindow()
{
    int w = Config::getInt("window","width",DEFAULT_WINDOW_WIDTH);
    int h = Config::getInt("window","height",DEFAULT_WINDOW_HEIGHT);
    bool fullscreen = Config::getBool("window","fullscreen",DEFAULT_WINDOW_FULLSCREEN);

    return m_renderWindow.create(w,h,m_createInfos.name,fullscreen);
}

bool VApp::createDummyAssets()
{
    unsigned char dummyTexturePtr[4] = {255,255,255,255};
    TextureHandler::instance()->enableDummyAsset();
    TextureAsset* dummyTexture = TextureHandler::instance()->getDummyAsset();

    return dummyTexture->generateTexture(dummyTexturePtr,1,1);
}

bool VApp::createDefaultRenderer()
{
    if(m_defaultRenderer != nullptr)
        delete m_defaultRenderer;

    m_defaultRenderer = new DefaultRenderer(&m_renderWindow);

    return (true);
}

bool VApp::createSceneRenderer()
{
    if(m_sceneRenderer != nullptr)
        delete m_sceneRenderer;

    m_sceneRenderer = new SceneRenderer(&m_renderWindow);

    return (true);
}

void VApp::loop()
{
    Clock clock;

    clock.restart();
    while(m_running)
    {
        Time elapsedTime = clock.restart();
        Profiler::resetLoop(ENABLE_PROFILER);

        Profiler::pushClock("Acquire next image");
        m_imageIndex = m_renderWindow.acquireNextImage();
        Profiler::popClock();

        Profiler::pushClock("Check buffer expansion");
        m_defaultRenderer->checkBuffersExpansion();
        Profiler::popClock();

        m_eventsManager.update();

        if(m_statesManager.peekState() == nullptr)
            this->stop();
        else {
            m_statesManager.handleEvents(&m_eventsManager);

            Profiler::pushClock("States update");
            m_statesManager.update(elapsedTime);
            Profiler::popClock();

            Profiler::pushClock("States draw");
            m_statesManager.draw(m_defaultRenderer);
            Profiler::popClock();
        }

        Profiler::pushClock("Draw frame");
        this->drawFrame();
        Profiler::popClock();
    }

    VInstance::waitDeviceIdle();
}

void VApp::drawFrame()
{
    Profiler::pushClock("Update buffers");
    m_defaultRenderer->updateBuffers(m_imageIndex);
    Profiler::popClock();

    Profiler::pushClock("Submit to queue");
    m_renderWindow.submitToGraphicsQueue(m_defaultRenderer->getCommandBuffer(),
                                         m_defaultRenderer->getRenderFinishedSemaphore(m_renderWindow.getCurrentFrameIndex()));
    Profiler::popClock();

    Profiler::pushClock("Present");
    m_renderWindow.display();
    Profiler::popClock();
}

void VApp::cleanup()
{
    if(m_sceneRenderer != nullptr)
    {
        delete m_sceneRenderer;
        m_sceneRenderer = nullptr;
    }

    if(m_defaultRenderer != nullptr)
    {
        delete m_defaultRenderer;
        m_defaultRenderer = nullptr;
    }

    TextureHandler::instance()->cleanAll();
    VMemoryAllocator::instance()->cleanAll();

    m_renderWindow.destroy();

    glfwTerminate();
}


}
