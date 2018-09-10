/**
* This class create the graphic pipeline necessary to render scenes.
**/

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Valag/gfx/AbstractRenderer.h"
#include "Valag/gfx/Scene.h"

namespace vlg
{

class SceneRenderer : public AbstractRenderer
{
    public:
        SceneRenderer(RenderWindow *targetWindow, RendererName name);
        virtual ~SceneRenderer();

        void update(size_t frameIndex);

        void draw(Scene* scene);

    protected:
        virtual bool init();
        virtual void cleanup();

        virtual bool    createRenderPass();
        virtual bool    createDescriptorSetLayouts();
        virtual bool    createGraphicsPipeline();
        virtual bool    createUBO();
        virtual bool    createDescriptorPool();
        virtual bool    createDescriptorSets();

        virtual bool    recordPrimaryCommandBuffer(uint32_t imageIndex);

    private:

};

}

#endif // SCENERENDERER_H
