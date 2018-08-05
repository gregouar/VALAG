#include "Valag/core/VApp.h"

#include "Valag/utils/Parser.h"
#include "Valag/utils/Logger.h"
#include "Valag/core/Config.h"

namespace vlg
{

const char *VApp::DEFAULT_APP_NAME = "VALAGEngine";
const char *VApp::DEFAULT_CONFIG_FILE = "config.ini";
const char *VApp::DEFAULT_SCREENSHOTPATH = "../screenshots/";

const char *VApp::DEFAULT_WINDOW_FULLSCREEN = "false";
const char *VApp::DEFAULT_WINDOW_WIDTH = "1024";
const char *VApp::DEFAULT_WINDOW_HEIGHT = "768";
const char *VApp::DEFAULT_VSYNC = "false";


const bool VApp::ENABLE_PROFILER = false;


VApp::VApp() : VApp(DEFAULT_APP_NAME)
{
    //ctor
}

VApp::VApp(const std::string& name) :
    m_name(name),
    m_vulkanInstance(nullptr),
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

    m_window = glfwCreateWindow(w,h,m_name.c_str(), monitor, nullptr);

    return (m_window != nullptr);
}

bool VApp::createVulkanInstance()
{
    if(m_vulkanInstance != nullptr)
        delete m_vulkanInstance;

    m_vulkanInstance = new VInstance(m_name);

    return (true);
}


void VApp::loop()
{
    while(m_running)
    {
        m_eventsManager.update();

        ///m_window.clear();

        if(m_statesManager.peekState() == nullptr)
            this->stop();
        else {
            m_statesManager.handleEvents(&m_eventsManager);

            m_statesManager.update(/**elapsed_time**/);

           /// m_stateManager.draw(&m_window);
        }
        ///m_window.display();
    }
}

void VApp::cleanup()
{
    if(m_vulkanInstance != nullptr)
    {
        delete m_vulkanInstance;
        m_vulkanInstance = nullptr;
    }

    glfwDestroyWindow(m_window);
    glfwTerminate();
}


}
