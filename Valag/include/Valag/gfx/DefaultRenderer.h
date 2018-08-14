#ifndef DEFAULTRENDERER_H
#define DEFAULTRENDERER_H

#include "Valag/gfx/VInstance.h"
#include "Valag/gfx/Sprite.h"

namespace vlg
{

class DefaultRenderer
{
    friend class VApp;

    public:
        DefaultRenderer(VInstance *vulkanInstance);
        virtual ~DefaultRenderer();

        void draw(Sprite *sprite);

        VkCommandBuffer getCommandBuffer(uint32_t imageIndex);
        VkSemaphore     getRenderFinishedSemaphore(size_t frameIndex);

    protected:
        bool init();

        bool    createTextureSampler();
        bool    createRenderPass();
        bool    createGraphicsPipeline();
        bool    createFramebuffers();
        bool    createPrimaryCommandBuffers();
        bool    createSemaphores();

        void    updateBuffers(uint32_t imageIndex);
        bool    recordPrimaryCommandBuffer(uint32_t imageIndex);

        void cleanup();

    private:
        VInstance  *m_vulkanInstance;

        VkSampler           m_defaultTextureSampler;
        VkRenderPass        m_defaultRenderPass;
        VkPipelineLayout    m_defaultPipelineLayout;
        VkPipeline          m_defaultPipeline;

        std::vector<VkFramebuffer>      m_swapchainFramebuffers;
        std::vector<VkCommandBuffer>    m_commandBuffers;

        std::vector<VkSemaphore>        m_renderFinishedSemaphore;

        std::vector<VkCommandBuffer>      m_activeSecondaryCommandBuffers;

        static const char *DEFAULT_VERTSHADERFILE;
        static const char *DEFAULT_FRAGSHADERFILE;
};

}

#endif // DEFAULTRENDERER_H
