#ifndef INSTANCINGRENDERER_H
#define INSTANCINGRENDERER_H

#include "Valag/renderers/AbstractRenderer.h"

namespace vlg
{

struct InstanciedVertex2D
{
    glm::vec3 pos;
    glm::vec4 color;
    glm::vec2 texCoord;
    glm::vec2 texId;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};

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
        static const char *INSTANCING_VERTSHADERFILE;
        static const char *INSTANCING_FRAGSHADERFILE;

        static const float  DEPTH_SCALING_FACTOR;
};

}

#endif // INSTANCINGRENDERER_H
