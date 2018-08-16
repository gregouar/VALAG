#ifndef DEFAULTRENDERER_H
#define DEFAULTRENDERER_H

#include "Valag/gfx/VInstance.h"
#include "Valag/gfx/Sprite.h"
#include "Valag/utils/DynamicUBO.h"

namespace vlg
{

class DefaultRenderer
{
    friend class VApp;
    friend class Sprite;

    public:
        DefaultRenderer(VInstance *vulkanInstance);
        virtual ~DefaultRenderer();

        void draw(Sprite *sprite);

        VkCommandBuffer getCommandBuffer();
        VkSemaphore     getRenderFinishedSemaphore(size_t frameIndex);

    protected:
        bool init();

        bool    createTextureSampler();
        bool    createRenderPass();
        bool    createDescriptorSetLayouts();
        bool    createGraphicsPipeline();
        bool    createFramebuffers();
        bool    createDescriptorPool();
        bool    createDescriptorSets();
        bool    createPrimaryCommandBuffers();
        bool    createSemaphores();

        bool    createUBO();
       // bool    createModelBuffers();
       // void    expandModelBuffers();

        void    updateBuffers(uint32_t imageIndex);
        void    updateViewUBO();
        bool    recordPrimaryCommandBuffer(uint32_t imageIndex);

        bool  allocModelUBO(size_t &index); ///Could use vector of indices
        void    updateModelUBO(size_t index, void *data, size_t frameIndex);
        void    updateModelDescriptorSets();

        void    bindAllUBOs(VkCommandBuffer &commandBuffer, size_t frameIndex, size_t modelUBOIndex);

        void cleanup();

    private:
        VInstance  *m_vulkanInstance;

        VkSampler               m_defaultTextureSampler;
        VkRenderPass            m_defaultRenderPass;
        VkPipelineLayout        m_defaultPipelineLayout;
        VkPipeline              m_defaultPipeline;

        std::vector<VkFramebuffer>      m_swapchainFramebuffers;
        std::vector<VkExtent2D>         m_swapchainExtents; //Could be needed if I implement resizing

        VkDescriptorPool                m_descriptorPool;
        std::vector<VkDescriptorSet>    m_descriptorSets;

        size_t                          m_currentCommandBuffer;
        std::vector<VkCommandBuffer>    m_commandBuffers;

        std::vector<VkSemaphore>        m_renderFinishedSemaphore;
        std::vector<VkCommandBuffer>    m_activeSecondaryCommandBuffers;

        std::vector<bool>               m_needToUpdateViewUBO;
        std::vector<VkBuffer>           m_viewBuffers; //Should be in a camera object for SceneRenderer
        std::vector<VkDeviceMemory>     m_viewBuffersMemory;
        std::vector<VkDescriptorSet>    m_viewDescriptorSets;
        VkDescriptorSetLayout           m_viewDescriptorSetLayout;

        /*std::vector<VkBuffer>           m_modelDynamicBuffers;
        std::vector<VkDeviceMemory>     m_modelDynamicBuffersMemory;*/
        std::vector<DynamicUBO*>        m_modelBuffers;
        std::vector<VkDescriptorSet>    m_modelDescriptorSets;
        VkDescriptorSetLayout           m_modelDescriptorSetLayout;




        static const char *DEFAULT_VERTSHADERFILE;
        static const char *DEFAULT_FRAGSHADERFILE;

        static const size_t MODEL_DYNAMICBUFFER_CHUNKSIZE;
};

}

#endif // DEFAULTRENDERER_H
