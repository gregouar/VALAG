#include "Valag/core/VApp.h"

#include "Valag/VulkanExtProxies.h"

#include "Valag/utils/Parser.h"
#include "Valag/utils/Logger.h"
#include "Valag/core/Config.h"

#include <cstring>

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

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


VApp::VApp() : VApp(DEFAULT_APP_NAME)
{
    //ctor
}

VApp::VApp(const std::string& name) :
    m_name(name),
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

    if(!this->initVulkan())
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

bool VApp::initVulkan()
{
    if(!this->createVulkanInstance())
        return (false);

    if(!this->setupDebugCallback())
        return (false);

    return (true);
}

bool VApp::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return (false);
    }

    return (true);
}

std::vector<const char*> VApp::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
}

bool VApp::createVulkanInstance()
{
    if (enableValidationLayers && !this->checkValidationLayerSupport())
        throw std::runtime_error("Validation layers requested not available");

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = m_name.c_str();
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "VALAG";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount    = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames  = validationLayers.data();
    } else {
        createInfo.enabledLayerCount    = 0;
    }

    return (vkCreateInstance(&createInfo, nullptr, &m_vulkanInstance) == VK_SUCCESS);
}

bool VApp::setupDebugCallback()
{
    if (!enableValidationLayers)
        return (true);

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = VApp::debugCallback;

    return (CreateDebugReportCallbackEXT(m_vulkanInstance, &createInfo, nullptr, &m_debugCallback) == VK_SUCCESS);
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
    if (enableValidationLayers)
        DestroyDebugReportCallbackEXT(m_vulkanInstance, m_debugCallback, nullptr);

    vkDestroyInstance(m_vulkanInstance, nullptr);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

VKAPI_ATTR VkBool32 VKAPI_CALL VApp::debugCallback(  VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                            uint64_t obj, size_t location, int32_t code,
                                                            const char* layerPrefix, const char* msg, void* userData)
{
    Logger::error(msg);

    return VK_FALSE;
}

}
