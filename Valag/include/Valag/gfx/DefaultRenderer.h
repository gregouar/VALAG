#ifndef DEFAULTRENDERER_H
#define DEFAULTRENDERER_H

#include "Valag/gfx/VInstance.h"

namespace vlg
{

class DefaultRenderer
{
    public:
        DefaultRenderer(VInstance *vulkanInstance);
        virtual ~DefaultRenderer();

        void render();

    protected:
        bool init();

        bool    createTextureSampler();
        bool    createRenderPass();
        bool    createGraphicsPipeline();
        bool    createFramebuffers();
        bool    createCommandBuffers();

        void cleanup();

    private:
        VInstance  *m_vulkanInstance;

        VkSampler           m_defaultTextureSampler;
        VkRenderPass        m_defaultRenderPass;
        VkPipelineLayout    m_defaultPipelineLayout;
        VkPipeline          m_defaultPipeline;

        std::vector<VkFramebuffer>      m_swapchainFramebuffers;
        std::vector<VkCommandBuffer>    m_commandBuffers;


        static const char *DEFAULT_VERTSHADERFILE;
        static const char *DEFAULT_FRAGSHADERFILE;
};

}

#endif // DEFAULTRENDERER_H
