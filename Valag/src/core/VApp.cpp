#include "Valag/core/VApp.h"

#include "Valag/utils/Parser.h"
#include "Valag/utils/Logger.h"
#include "Valag/utils/Clock.h"
#include "Valag/core/Config.h"

#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"
#include "Valag/vulkanImpl/VBuffersAllocator.h"

#include "Valag/renderers/DefaultRenderer.h"
#include "Valag/renderers/SceneRenderer.h"
#include "Valag/renderers/InstancingRenderer.h"

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
const size_t VApp::MAX_FRAMES_IN_FLIGHT = 3;

VApp::VApp(const VAppCreateInfos &infos) :
    m_createInfos(infos),
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

 //   Profiler::resetLoop(ENABLE_PROFILER);

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

    std::srand(std::time(nullptr));

    Logger::write("Initializing application");

    glfwInit();

    Profiler::pushClock("Create window");
    if(!this->createWindow())
        throw std::runtime_error("Cannot create window");
    Profiler::popClock();

    Profiler::pushClock("Init vulkan instance");
    VInstance::instance()->init(m_renderWindow.getSurface()); //Throw error
    Profiler::popClock();

    VTexturesManager::instance()->init(m_renderWindow.getFramesCount());

    if(!m_renderWindow.init())
        throw std::runtime_error("Cannot initialize window");

    m_eventsManager.init(m_renderWindow.getWindowPtr());

    Profiler::pushClock("Create renderers");
    if(!this->createRenderers())
        throw std::runtime_error("Cannot create renderers");
    /*if(!this->createDefaultRenderer())
        throw std::runtime_error("Cannot create default renderer");

    if(!this->createSceneRenderer())
        throw std::runtime_error("Cannot create scene renderer");*/
    Profiler::popClock();

    return (true);
}

bool VApp::createWindow()
{
    int w = Config::getInt("window","width",DEFAULT_WINDOW_WIDTH);
    int h = Config::getInt("window","height",DEFAULT_WINDOW_HEIGHT);
    bool fullscreen = Config::getBool("window","fullscreen",DEFAULT_WINDOW_FULLSCREEN);
    size_t framesCount = MAX_FRAMES_IN_FLIGHT;

    return m_renderWindow.create(w,h,m_createInfos.name,fullscreen, framesCount);
}

bool VApp::createRenderers()
{
    for(auto renderer : m_renderers)
        delete renderer;
    m_renderers.clear();

    //m_renderers.push_back(new DefaultRenderer(&m_renderWindow, Renderer_Default, Renderer_Unique));
    //m_renderWindow.attachRenderer(m_renderers.back());

    m_renderers.push_back(new SceneRenderer(&m_renderWindow, Renderer_Scene, Renderer_First));
   // m_renderWindow.attachRenderer(m_renderers.back());

    m_renderers.push_back(new InstancingRenderer(&m_renderWindow, Renderer_Instancing,Renderer_Unique));
    m_renderWindow.attachRenderer(m_renderers.back());

    return (true);
}

/*bool VApp::createDefaultRenderer()
{
    if(m_defaultRenderer != nullptr)
        delete m_defaultRenderer;

    m_defaultRenderer = new DefaultRenderer(&m_renderWindow, Renderer_Default);
    m_renderWindow.attachRenderer(m_defaultRenderer);

    return (true);
}

bool VApp::createSceneRenderer()
{
    if(m_sceneRenderer != nullptr)
        delete m_sceneRenderer;

    m_sceneRenderer = new SceneRenderer(&m_renderWindow, Renderer_Scene);
    m_renderWindow.attachRenderer(m_sceneRenderer);

    return (true);
}*/

void VApp::loop()
{
    Clock clock;

    clock.restart();
    while(m_running)
    {
        Time elapsedTime = clock.restart();
        Profiler::resetLoop(ENABLE_PROFILER);

        Profiler::pushClock("Acquire next image");
        m_renderWindow.acquireNextImage();
        Profiler::popClock();

        VTexturesManager::instance()->checkUpdateDescriptorSets(m_renderWindow.getCurrentFrameIndex());

        m_eventsManager.update();

        if(m_statesManager.peekState() == nullptr)
            this->stop();
        else {
            m_statesManager.handleEvents(&m_eventsManager);

            Profiler::pushClock("States update");
            m_statesManager.update(elapsedTime);
            Profiler::popClock();

            Profiler::pushClock("States draw");
            m_statesManager.draw(&m_renderWindow);
            Profiler::popClock();
        }

        Profiler::pushClock("Display");
        m_renderWindow.display();
        Profiler::popClock();
    }

    VInstance::waitDeviceIdle();
}

void VApp::cleanup()
{
    for(auto renderer : m_renderers)
        delete renderer;
    m_renderers.clear();

    TexturesHandler::instance()->cleanAll();
    VBuffersAllocator::instance()->cleanAll();

    m_renderWindow.destroy();

    glfwTerminate();
}


}
