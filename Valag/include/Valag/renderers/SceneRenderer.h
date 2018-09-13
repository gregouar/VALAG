/**
* This class create the graphic pipeline necessary to render scenes.
**/

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Valag/renderers/AbstractRenderer.h"
#include "Valag/scene/Scene.h"

namespace vlg
{

class SceneRenderer : public AbstractRenderer
{
    public:
        SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order);
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
        ///Should I do a pipeline for alpha and one for opacity ?

        VkPipelineLayout    m_deferredPipelineLayout,
                            m_compositionPipelineLayout;
        VkPipeline          m_deferredPipeline,
                            m_compositionPipeline;



        ///The default pipeline is used for bloom and tone mapping

};

}

#endif // SCENERENDERER_H
