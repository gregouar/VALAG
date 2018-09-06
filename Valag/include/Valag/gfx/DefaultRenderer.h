#ifndef DEFAULTRENDERER_H
#define DEFAULTRENDERER_H

#include "Valag/vulkanImpl/RenderWindow.h"
#include "Valag/vulkanImpl/DynamicUBO.h"
#include "Valag/vulkanImpl/VTexturesManager.h"
#include "Valag/gfx/Drawable.h"


namespace vlg
{

class DefaultRenderer
{
    friend class VApp;
    friend class Sprite;
    friend class SpritesBatch;

    public:
        DefaultRenderer(RenderWindow *targetWindow);
        virtual ~DefaultRenderer();

        void draw(Drawable *drawable);

        VkCommandBuffer getCommandBuffer();
        VkSemaphore     getRenderFinishedSemaphore(size_t frameIndex);

    protected:
        bool init();

        bool    createRenderPass();
        bool    createDescriptorSetLayouts();
        bool    createGraphicsPipeline();
        bool    createFramebuffers();
        bool    createDescriptorPool();
        bool    createDescriptorSets();
        bool    createPrimaryCommandBuffers();
        bool    createSemaphores();

        bool    createUBO();

        void    updateBuffers(uint32_t imageIndex);
        void    updateViewUBO();
        bool    recordPrimaryCommandBuffer(uint32_t imageIndex);

        //bool    allocModelUBO(size_t &index, size_t frameIndex); ///Could use vector of indices
        //void    updateModelUBO(size_t index, void *data, size_t frameIndex);
        //void    updateModelDescriptorSets(size_t frameIndex);
        //void    checkBuffersExpansion();

        //size_t  getModelUBOBufferVersion(size_t frameIndex);
        size_t  getTextureArrayDescSetVersion(size_t frameIndex); /// I should remove it to directly use VTextureManager

        void    bindDefaultPipeline(VkCommandBuffer &commandBuffer, size_t frameIndex);
        bool    bindTexture(VkCommandBuffer &commandBuffer, AssetTypeID textureID, size_t frameIndex);
        //void    bindModelUBO(VkCommandBuffer &commandBuffer, size_t frameIndex, size_t modelUBOIndex);
        void    bindModelDescriptorSet(VkCommandBuffer &commandBuffer, VkDescriptorSet descSet, uint32_t dynamicOffset = 0);

        void cleanup();

    private:
        RenderWindow  *m_targetWindow;

        VkRenderPass            m_defaultRenderPass;
        VkPipelineLayout        m_defaultPipelineLayout;
        VkPipeline              m_defaultPipeline;

        std::vector<VkFramebuffer>      m_swapchainFramebuffers;
        std::vector<VkExtent2D>         m_swapchainExtents; //Could be needed if I implement resizing

        VkDescriptorPool                m_descriptorPool;

        size_t                          m_curFrameIndex;
        size_t                          m_lastFrameIndex;
        std::vector<VkCommandBuffer>    m_commandBuffers;

        std::vector<VkSemaphore>        m_renderFinishedSemaphore;
        std::vector<VkCommandBuffer>    m_activeSecondaryCommandBuffers;

        ///I will probably switch all this to camera... and maybe have something similar to sprites (with dynamic ubo and so on ?)
        std::vector<bool>               m_needToUpdateViewUBO;
        std::vector<VBuffer>            m_viewBuffers; //Should be in a camera object for SceneRenderer
        std::vector<VkDescriptorSet>    m_viewDescriptorSets;
        VkDescriptorSetLayout           m_viewDescriptorSetLayout;

        /*std::vector<bool>               m_needToExpandModelBuffers;
        std::vector<DynamicUBO*>        m_modelBuffers;
        std::vector<VkDescriptorSet>    m_modelDescriptorSets;
        VkDescriptorSetLayout           m_modelDescriptorSetLayout;*/

        static const char *DEFAULT_VERTSHADERFILE;
        static const char *DEFAULT_FRAGSHADERFILE;

        //static const size_t MODEL_DYNAMICBUFFER_CHUNKSIZE;
        static const float  DEPTH_SCALING_FACTOR;
};

}

#endif // DEFAULTRENDERER_H
