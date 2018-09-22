#ifndef DEFAULTRENDERER_H
#define DEFAULTRENDERER_H

#include "Valag/renderers/AbstractRenderer.h"
#include "Valag/vulkanImpl/DynamicUBODescriptor.h"
#include "Valag/vulkanImpl/VTexturesManager.h"
#include "Valag/gfx/Drawable.h"


namespace vlg
{

class DefaultRenderer : public AbstractRenderer
{
    friend class VApp;
    friend class Sprite;
    friend class SpritesBatch;

    public:
        DefaultRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order);
        virtual ~DefaultRenderer();

        virtual void update(size_t frameIndex);

        void draw(Drawable *drawable);

    protected:
        virtual bool init();
        virtual void cleanup();

        virtual bool    createDescriptorSetLayouts();
        virtual bool    createGraphicsPipeline();
        virtual bool    createDescriptorPool();
        virtual bool    createDescriptorSets();

        virtual bool    recordPrimaryCmb(uint32_t imageIndex);

        void    bindPipeline(VkCommandBuffer &commandBuffer, size_t frameIndex);
        bool    bindTexture(VkCommandBuffer &commandBuffer, AssetTypeID textureID, size_t frameIndex);
        void    bindModelDescriptorSet(size_t frameIndex, VkCommandBuffer &commandBuffer, DynamicUBODescriptor &descSet, size_t index = 0);


    private:
        std::vector<VkCommandBuffer>    m_activeSecondaryCommandBuffers;

        VGraphicsPipeline m_pipeline;

        static const char *DEFAULT_VERTSHADERFILE;
        static const char *DEFAULT_FRAGSHADERFILE;

        static const float  DEPTH_SCALING_FACTOR;
};

}

#endif // DEFAULTRENDERER_H
