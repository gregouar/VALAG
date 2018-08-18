/**
* This class create the graphic pipeline necessary to render scenes.
**/

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Valag/vulkanImpl/VInstance.h"

namespace vlg
{

class SceneRenderer
{
    public:
        SceneRenderer(VInstance *vulkanInstance);
        virtual ~SceneRenderer();

    protected:
        void init();
        void cleanup();

    private:
        VInstance *m_vulkanInstance;
};

}

#endif // SCENERENDERER_H
