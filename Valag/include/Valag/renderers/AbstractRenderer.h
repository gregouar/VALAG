#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "Valag/renderers/RenderWindow.h"
#include "Valag/renderers/RenderView.h"
#include "Valag/renderers/RenderGraph.h"

namespace vlg
{

class AbstractRenderer
{
    public:
        AbstractRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order);
        virtual ~AbstractRenderer();

        virtual void update(size_t frameIndex);
        virtual void render(uint32_t imageIndex);

        virtual void setView(glm::mat4 view, glm::mat4 viewInv);

        virtual std::vector<FullRenderPass*> getFinalPasses();

        RendererName    getName();

    protected:
        virtual void    prepareRenderPass();

        virtual bool    initRenderGraph();
        virtual bool    createGraphicsPipeline() = 0;
        virtual bool    createRenderView();

        virtual bool    createDescriptorSetLayouts();
        virtual bool    createDescriptorPool();
        virtual bool    createDescriptorSets();

        virtual bool    init();
        virtual void    cleanup();

        virtual bool    recordPrimaryCmb(uint32_t imageIndex) = 0;


    protected:
        RenderWindow   *m_targetWindow;
        RenderView      m_renderView;
        RenderGraph     m_renderGraph;
        size_t          m_defaultPass;

        std::vector<VkDescriptorPoolSize>   m_descriptorPoolSizes;
        VkDescriptorPool                    m_descriptorPool;

        size_t                          m_curFrameIndex;

    private:
        RenderereOrder  m_order;
        RendererName    m_name;

        std::vector<FullRenderPass*> m_finalPasses;
};

}

#endif // ABSTRACTRENDERER_H
