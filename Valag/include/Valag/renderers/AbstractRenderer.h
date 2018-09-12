#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "Valag/renderers/RenderWindow.h"
#include "Valag/renderers/RenderView.h"

namespace vlg
{

class AbstractRenderer
{
    public:
        AbstractRenderer(RenderWindow *targetWindow, RendererName name);
        virtual ~AbstractRenderer();

        virtual void update(size_t frameIndex);
        virtual void updateCMB(uint32_t imageIndex);

        VkCommandBuffer getCommandBuffer(size_t frameIndex);
        //VkSemaphore     getRenderFinishedSemaphore(size_t frameIndex);

        RendererName    getName();

    protected:
        virtual bool    createRenderPass() = 0;
        virtual bool    createDescriptorSetLayouts() = 0;
        virtual bool    createGraphicsPipeline() = 0;
        virtual bool    createSwapchainFramebuffers();
        virtual bool    createRenderView();
        virtual bool    createUBO() = 0;
        virtual bool    createDescriptorPool() = 0;
        virtual bool    createDescriptorSets() = 0;
        virtual bool    createPrimaryCommandBuffers();
        //virtual bool    createSemaphores();

        virtual bool    init();
        virtual void    cleanup();

        virtual bool    recordPrimaryCommandBuffer(uint32_t imageIndex) = 0;


    protected:
        RenderWindow  *m_targetWindow;

        VkRenderPass        m_renderPass;
        VkPipelineLayout    m_pipelineLayout;
        VkPipeline          m_pipeline;

        VkDescriptorPool                m_descriptorPool;

        std::vector<VkFramebuffer>      m_swapchainFramebuffers;
        std::vector<VkExtent2D>         m_swapchainExtents; //Could be needed if I implement resizing

        size_t                          m_curFrameIndex;
        std::vector<VkCommandBuffer>    m_primaryCMB;

        RenderView  m_renderView;

        //std::vector<VkSemaphore>        m_renderFinishedSemaphore;
        //std::vector<VkCommandBuffer>    m_activeSecondaryCommandBuffers;

    private:
        RendererName m_name;
};

}

#endif // ABSTRACTRENDERER_H
