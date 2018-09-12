#ifndef VAPP_H
#define VAPP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Valag/core/GameState.h"
#include "Valag/core/EventsManager.h"
#include "Valag/core/StatesManager.h"

#include "Valag/renderers/RenderWindow.h"
#include "Valag/renderers/AbstractRenderer.h"

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
        VApp(const VAppCreateInfos &infos);
        virtual ~VApp();


        void run(GameState *startingState);
        void stop();

        void printscreen();

    protected:
        bool    init();

        bool    createWindow();
        bool    createRenderers();

        void    loop();

        void    cleanup();

    private:
        bool        m_running;

        VAppCreateInfos     m_createInfos;

        StatesManager       m_statesManager;
        EventsManager       m_eventsManager;
        RenderWindow        m_renderWindow;

        /*DefaultRenderer    *m_defaultRenderer;
        SceneRenderer      *m_sceneRenderer;
        InstancingRenderer *m_instancingRenderer;*/

        std::vector<AbstractRenderer*> m_renderers;

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

