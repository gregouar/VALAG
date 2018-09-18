/**
* This class create the graphic pipeline necessary to render scenes.
**/

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "Valag/renderers/AbstractRenderer.h"

#include "Valag/scene/Scene.h"
#include "Valag/scene/IsoSpriteEntity.h"

#include "Valag/vulkanImpl/DynamicVBO.h"

namespace vlg
{

class SceneRenderer : public AbstractRenderer
{
    public:
        SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order);
        virtual ~SceneRenderer();

        void update(size_t frameIndex);

        void draw(Scene* scene);
        void draw(IsoSpriteEntity* sprite);

    protected:
        virtual bool init();
        virtual void cleanup();

        virtual bool    createRenderPass();
        virtual bool    createDescriptorSetLayouts();
        virtual bool    createGraphicsPipeline();
        virtual bool    createFramebuffers();
        virtual bool    createUBO();
        virtual bool    createDescriptorPool();
        virtual bool    createDescriptorSets();

        bool createAttachments();

        bool createDeferredFramebuffers();
        bool createAmbientLightingFramebuffers();

        bool createDeferredRenderPass();
        bool createAmbientLightingRenderPass();

        bool createDeferredPipeline();
        bool createAmbientLightingPipeline();
        bool createToneMappingPipeline();

        bool createSemaphores();

        virtual bool    recordPrimaryCommandBuffer(uint32_t imageIndex);

    private:
        ///Should I do a pipeline for alpha and one for opacity or could I play with descriptors ?


        VkDescriptorSetLayout           m_deferredDescriptorSetLayout,
                                        m_hdrDescriptorSetLayout;
        VkDescriptorPool                m_descriptorPool;
        std::vector<VkDescriptorSet>    m_deferredDescriptorSets,
                                        m_hdrDescriptorSets;

        std::vector<VkFramebuffer>  m_deferredFramebuffers,
                                    m_ambientLightingFramebuffers;

        VkRenderPass m_deferredRenderPass,
                     m_ambientLightingRenderPass;

        VGraphicsPipeline   m_deferredPipeline,
                            m_ambientLightingPipeline,
                            m_toneMappingPipeline;

        VkSampler m_attachmentsSampler;
        std::vector<VFramebufferAttachment> m_albedoAttachments,
                                            m_heightAttachments,
                                            m_normalAttachments,
                                            m_rmtAttachments,
                                            m_hdrAttachements;

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
