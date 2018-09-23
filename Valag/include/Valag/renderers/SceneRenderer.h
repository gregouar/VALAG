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

#define NBR_ALPHA_LAYERS 2

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
        virtual bool    createGraphicsPipeline();

        bool createAttachments();

        bool createDeferredRenderPass();
        bool createAlphaDetectRenderPass();
        bool createAmbientLightingRenderPass();
        bool createToneMappingRenderPass();

        bool createDeferredPipeline();
        bool createAlphaDetectPipeline();
        bool createAmbientLightingPipeline();
        bool createToneMappingPipeline();

        virtual bool    recordPrimaryCmb(uint32_t imageIndex); ///Deferred, Alpha Detect, Alpha deferred
        virtual bool    recordAmbientLightingCmb(uint32_t imageIndex);
        virtual bool    recordToneMappingCmb(uint32_t imageIndex);

    private:
        ///Should I do a pipeline for alpha and one for opacity or could I play with descriptors ?

        VGraphicsPipeline   m_deferredPipeline,
                            m_alphaDetectPipeline,
                            m_ambientLightingPipeline,
                            m_toneMappingPipeline;

        //VkSampler m_attachmentsSampler;
        std::vector<VFramebufferAttachment> m_deferredDepthAttachments; ///Could use multiple if I want to implement multiple depth layers
        std::vector<VFramebufferAttachment> m_albedoAttachments[NBR_ALPHA_LAYERS],
                                            m_positionAttachments[NBR_ALPHA_LAYERS],
                                            m_normalAttachments[NBR_ALPHA_LAYERS],
                                            m_rmtAttachments[NBR_ALPHA_LAYERS];
        std::vector<VFramebufferAttachment> m_alphaDetectAttachments;
        std::vector<VFramebufferAttachment> m_hdrAttachements[NBR_ALPHA_LAYERS];

        size_t  m_deferredPass,
                m_alphaDetectPass,
                m_alphaDeferredPass,
                m_ambientLightingPass,
                m_toneMappingPass;

        std::vector<DynamicVBO<InstanciedIsoSpriteDatum> >    m_spritesVbos;

        static const char *ISOSPRITE_DEFERRED_VERTSHADERFILE;
        static const char *ISOSPRITE_DEFERRED_FRAGSHADERFILE;
        static const char *ISOSPRITE_ALPHADETECT_VERTSHADERFILE;
        static const char *ISOSPRITE_ALPHADETECT_FRAGSHADERFILE;
        static const char *AMBIENTLIGHTING_VERTSHADERFILE;
        static const char *AMBIENTLIGHTING_FRAGSHADERFILE;
        static const char *TONEMAPPING_VERTSHADERFILE;
        static const char *TONEMAPPING_FRAGSHADERFILE;
};

}

#endif // SCENERENDERER_H
