/**
* This class create the graphic pipeline necessary to render scenes.
**/

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Valag/vulkanImpl/RenderWindow.h"
#include "Valag/gfx/Scene.h"

namespace vlg
{

class SceneRenderer
{
    public:
        SceneRenderer(RenderWindow *targetWindow);
        virtual ~SceneRenderer();

        void draw(Scene* scene);

    protected:
        void init();
        void cleanup();

    private:
        RenderWindow *m_targetWindow; //Will probably be not needed for SceneRenderer ?
};

}

#endif // SCENERENDERER_H
