#ifndef VAPP_H
#define VAPP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Valag/core/GameState.h"
#include "Valag/core/EventsManager.h"
#include "Valag/core/StatesManager.h"
#include "Valag/gfx/VInstance.h"
#include "Valag/gfx/DefaultRenderer.h"
#include "Valag/gfx/SceneRenderer.h"

namespace vlg
{

struct VAppCreateInfos
{
    std::string name;

    float xyAngle;
    float zAngle;
};

class VApp
{
    public:
        //VApp();
        VApp(const VAppCreateInfos &infos);
        virtual ~VApp();


        void run(GameState *startingState);
        void stop();

        void printscreen();

        //sf::Vector2u getWindowSize();
        //const std::string &getName();

    protected:
        bool    init();

        bool    checkVideoMode(int w, int h, GLFWmonitor *monitor);
        bool    createWindow();
        bool    createVulkanInstance();
        bool    createDefaultRenderer();
        bool    createSceneRenderer();

        void    loop();
        void    drawFrame();

        void    cleanup();

    private:
        bool        m_running;
        //std::string m_name;
        VAppCreateInfos     m_createInfos;

        StatesManager       m_statesManager;
        EventsManager       m_eventsManager;
        GLFWwindow         *m_window;
        VInstance          *m_vulkanInstance;
        DefaultRenderer    *m_defaultRenderer;
        SceneRenderer      *m_sceneRenderer;

        unsigned int m_sceenshotNbr;

    public:
        static const char *DEFAULT_APP_NAME;
        static const char *DEFAULT_CONFIG_FILE;
        static const char *DEFAULT_SCREENSHOTPATH;
        static const char *DEFAULT_SHADERPATH;

        static const char *DEFAULT_WINDOW_FULLSCREEN;
        static const char *DEFAULT_WINDOW_WIDTH;
        static const char *DEFAULT_WINDOW_HEIGHT;
        static const char *DEFAULT_VSYNC;
        static const char *DEFAULT_ANISOTROPIC;

        static const bool ENABLE_PROFILER;
        static const size_t MAX_FRAMES_IN_FLIGHT;
};

}

#endif // VAPP_H

