#ifndef INSTANCINGRENDERER_H
#define INSTANCINGRENDERER_H

#include "Valag/renderers/AbstractRenderer.h"

#include "Valag/gfx/Sprite.h"
#include "Valag/vulkanImpl/DynamicVBO.h"

namespace vlg
{

class SpritesBatch;

struct InstanciedSpriteDatum
{
    glm::vec4 model_0;
    glm::vec4 model_1;
    glm::vec4 model_2;
    glm::vec4 model_3;
    glm::vec4 color;
    glm::vec2 texPos;
    glm::vec2 texExtent;
    glm::vec2 texId;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 8> getAttributeDescriptions();
};



class InstancingRenderer : public AbstractRenderer
{
    public:
        InstancingRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order);
        virtual ~InstancingRenderer();

        void update(size_t frameIndex);

        void draw(Sprite* sprite);
        void draw(SpritesBatch* spritesBatch);

    protected:
        virtual bool init();
        virtual void cleanup();

        //virtual bool    createRenderPass();
        virtual bool    createDescriptorSetLayouts();
        virtual bool    createGraphicsPipeline();
        virtual bool    createUBO();
        virtual bool    createDescriptorPool();
        virtual bool    createDescriptorSets();

        virtual bool    recordPrimaryCmb(uint32_t imageIndex);

    protected:
        std::vector<DynamicVBO<InstanciedSpriteDatum> > m_spritesVbos;

        VGraphicsPipeline m_pipeline;

    private:
        static const char *INSTANCING_VERTSHADERFILE;
        static const char *INSTANCING_FRAGSHADERFILE;

        static const float  DEPTH_SCALING_FACTOR;
};

}

#endif // INSTANCINGRENDERER_H
