#ifndef INSTANCINGRENDERER_H
#define INSTANCINGRENDERER_H

#include "Valag/renderers/AbstractRenderer.h"

namespace vlg
{

class InstancingRenderer : public AbstractRenderer
{
    public:
        InstancingRenderer(RenderWindow *targetWindow, RendererName name);
        virtual ~InstancingRenderer();

        void update(size_t frameIndex);

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

#endif // INSTANCINGRENDERER_H
