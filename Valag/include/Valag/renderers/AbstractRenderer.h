#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "Valag/renderers/RenderWindow.h"
#include "Valag/renderers/RenderView.h"

namespace vlg
{

class AbstractRenderer
{
    public:
        AbstractRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order);
        virtual ~AbstractRenderer();

        virtual void update(size_t frameIndex);
        virtual void updateCmb(uint32_t imageIndex);

        virtual VkCommandBuffer getCommandBuffer(size_t frameIndex, size_t imageIndex);
        //VkSemaphore     getRenderFinishedSemaphore(size_t frameIndex);
        virtual VkSemaphore     getFinalPassWaitSemaphore(size_t frameIndex);

        RendererName    getName();

    protected:
        virtual bool    createRenderPass();
        virtual bool    createDescriptorSetLayouts() = 0;
        virtual bool    createGraphicsPipeline() = 0;
        virtual bool    createFramebuffers();
        virtual bool    createRenderView();
        virtual bool    createUBO() = 0;
        virtual bool    createDescriptorPool() = 0;
        virtual bool    createDescriptorSets() = 0;
        virtual bool    createPrimaryCmb();
        //virtual bool    createSemaphores();

        virtual bool    init();
        virtual void    cleanup();

        virtual bool    recordPrimaryCmb(uint32_t imageIndex) = 0;


    protected:
        RenderWindow  *m_targetWindow;

        VkRenderPass        m_renderPass;

        VkDescriptorPool                m_descriptorPool;

        std::vector<VkFramebuffer>      m_framebuffers;
        std::vector<VkExtent2D>         m_swapchainExtents; //Could be needed if I implement resizing

        size_t                          m_curFrameIndex;
        std::vector<VkCommandBuffer>    m_primaryCmb;

        RenderView  m_renderView;

        RenderereOrder  m_order;

    private:
        RendererName    m_name;
};

}

#endif // ABSTRACTRENDERER_H
