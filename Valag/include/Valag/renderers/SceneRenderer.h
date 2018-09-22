/**
* This class create the graphic pipeline necessary to render scenes.
**/

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Valag/renderers/AbstractRenderer.h"
#include "Valag/renderers/RenderGraph.h"

#include "Valag/scene/Scene.h"
#include "Valag/scene/IsoSpriteEntity.h"

#include "Valag/vulkanImpl/DynamicVBO.h"

#define NBR_ALPHA_LAYERS = 2

namespace vlg
{

class SceneRenderer : public AbstractRenderer
{
    public:
        SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order);
        virtual ~SceneRenderer();

        void addToSpritesVbo(const InstanciedIsoSpriteDatum &datum);

    protected:
        virtual bool init();
        virtual void cleanup();

        virtual bool    createRenderPass();
        virtual bool    createDescriptorSetLayouts();
        virtual bool    createGraphicsPipeline();
        virtual bool    createDescriptorPool();
        virtual bool    createDescriptorSets();

        bool createAttachments();

        bool createDeferredRenderPass();
        bool createAmbientLightingRenderPass();
        bool createToneMappingRenderPass();

        bool createDeferredPipeline();
        bool createAmbientLightingPipeline();
        bool createToneMappingPipeline();

        virtual bool    recordPrimaryCmb(uint32_t imageIndex); ///Deferred
        virtual bool    recordAmbientLightingCmb(uint32_t imageIndex);
        virtual bool    recordToneMappingCmb(uint32_t imageIndex);

    private:
        ///Should I do a pipeline for alpha and one for opacity or could I play with descriptors ?

        VkDescriptorSetLayout           m_deferredDescriptorSetLayout,
                                        m_hdrDescriptorSetLayout;
        VkDescriptorPool                m_descriptorPool;
        std::vector<VkDescriptorSet>    m_deferredDescriptorSets,
                                        m_hdrDescriptorSets;

        VGraphicsPipeline   m_deferredPipeline,
                            m_ambientLightingPipeline,
                            m_toneMappingPipeline;

        VkSampler m_attachmentsSampler;
        std::vector<VFramebufferAttachment> m_albedoAttachments[NBR_ALPHA_LAYERS],
                                            m_heightAttachments[NBR_ALPHA_LAYERS],
                                            m_normalAttachments[NBR_ALPHA_LAYERS],
                                            m_rmtAttachments[NBR_ALPHA_LAYERS],
                                            m_hdrAttachements[NBR_ALPHA_LAYERS];
        size_t m_deferredPass;
        size_t m_ambientLightingPass;
        size_t m_toneMappingPass;

        std::vector<DynamicVBO<InstanciedIsoSpriteDatum> >    m_spritesVbos;

        static const char *ISOSPRITE_VERTSHADERFILE;
        static const char *ISOSPRITE_FRAGSHADERFILE;
        static const char *AMBIENTLIGHTING_VERTSHADERFILE;
        static const char *AMBIENTLIGHTING_FRAGSHADERFILE;
        static const char *TONEMAPPING_VERTSHADERFILE;
        static const char *TONEMAPPING_FRAGSHADERFILE;
};

}

#endif // SCENERENDERER_H
