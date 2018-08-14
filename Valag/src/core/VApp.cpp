#include "Valag/core/VApp.h"

#include "Valag/utils/Parser.h"
#include "Valag/utils/Logger.h"
#include "Valag/utils/Clock.h"
#include "Valag/core/Config.h"

#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"

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

const size_t VApp::MAX_FRAMES_IN_FLIGHT = 2;


/*VApp::VApp() : VApp(DEFAULT_APP_NAME)
{
    //ctor
}*/

VApp::VApp(const VAppCreateInfos &infos) :
    m_createInfos(infos),
    m_window(nullptr),
    m_vulkanInstance(nullptr),
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

    m_eventsManager.init(m_window);

    if(!this->createVulkanInstance())
        throw std::runtime_error("Cannot initialize Vulkan");

    m_vulkanInstance->setActive();

    //if(! TexturesHandler::instance()->createCommandPool())
       // throw std::runtime_error("Cannot create command pool for texturesHandler");

    if(!this->createDefaultRenderer())
        throw std::runtime_error("Cannot create default renderer");

    if(!this->createSceneRenderer())
        throw std::runtime_error("Cannot create scene renderer");

    return (true);
}

bool VApp::checkVideoMode(int w, int h, GLFWmonitor *monitor)
{
    int count;
    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

    bool ok = false;

    for(auto i = 0 ; i < count ; ++i)
    {
        if(modes[i].width == w && modes[i].height == h)
            ok = true;
    }

    return ok;
}

bool VApp::createWindow()
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    int w = Config::getInt("window","width",DEFAULT_WINDOW_WIDTH);
    int h = Config::getInt("window","height",DEFAULT_WINDOW_HEIGHT);

    if(!this->checkVideoMode(w,h,monitor))
    {
        std::ostringstream error_report;
        error_report<<"Invalid resolution "<<w<<"x"<<h;
        Logger::error(error_report);

        w = Parser::parseInt(DEFAULT_WINDOW_WIDTH);
        h = Parser::parseInt(DEFAULT_WINDOW_HEIGHT);

        if(!this->checkVideoMode(w,h,monitor))
        {
            std::ostringstream error_report;
            error_report<<"Invalid default resolution "<<w<<"x"<<h;
            Logger::error(error_report);

            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            w = mode->width;
            h = mode->height;
        }
    }

    if(!Config::getBool("window","fullscreen",DEFAULT_WINDOW_FULLSCREEN))
        monitor = nullptr;

    m_window = glfwCreateWindow(w,h,m_createInfos.name.c_str(), monitor, nullptr);

    return (m_window != nullptr);
}

bool VApp::createVulkanInstance()
{
    if(m_vulkanInstance != nullptr)
        delete m_vulkanInstance;

    m_vulkanInstance = new VInstance(m_window,m_createInfos.name);

    return (true);
}

bool VApp::createDefaultRenderer()
{
    if(m_defaultRenderer != nullptr)
        delete m_defaultRenderer;

    m_defaultRenderer = new DefaultRenderer(m_vulkanInstance);

    return (true);
}

bool VApp::createSceneRenderer()
{
    if(m_sceneRenderer != nullptr)
        delete m_sceneRenderer;

    m_sceneRenderer = new SceneRenderer(m_vulkanInstance);

    return (true);
}

void VApp::loop()
{
    Clock clock;

    clock.restart();
    while(m_running)
    {
        Time elapsedTime = clock.restart();

        m_eventsManager.update();

        ///m_window.clear();

        if(m_statesManager.peekState() == nullptr)
            this->stop();
        else {
            m_statesManager.handleEvents(&m_eventsManager);

            m_statesManager.update(elapsedTime);

            this->drawFrame();

           /// m_stateManager.draw(&m_window);
        }
        ///m_window.display();
    }

    m_vulkanInstance->waitDeviceIdle();
}

void VApp::drawFrame()
{
    uint32_t imageIndex = m_vulkanInstance->acquireNextImage();

    m_defaultRenderer->updateBuffers(imageIndex);

    m_vulkanInstance->submitToGraphicsQueue(m_defaultRenderer->getCommandBuffer(imageIndex),
                                            m_defaultRenderer->getRenderFinishedSemaphore(m_vulkanInstance->getCurrentFrameIndex()));

    m_vulkanInstance->presentQueue();
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

    if(m_vulkanInstance != nullptr)
    {
        delete m_vulkanInstance;
        m_vulkanInstance = nullptr;
    }

    if(m_window != nullptr)
        glfwDestroyWindow(m_window);

    glfwTerminate();
}


}
