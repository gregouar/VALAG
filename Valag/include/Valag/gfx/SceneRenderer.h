/**
* This class create the graphic pipeline necessary to render scenes.
**/

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Valag/vulkanImpl/RenderWindow.h"

namespace vlg
{

class SceneRenderer
{
    public:
        SceneRenderer(RenderWindow *targetWindow);
        virtual ~SceneRenderer();

    protected:
        void init();
        void cleanup();

    private:
        RenderWindow *m_targetWindow;
};

}

#endif // SCENERENDERER_H
