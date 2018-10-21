#ifndef FULLRENDERPASS_H
#define FULLRENDERPASS_H

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

//Need to add: addRenderTarget and createRenderTarger !

class FullRenderPass
{
    public:
        FullRenderPass(size_t imagesCount, size_t framesCount, bool useDynamicRenderTargets = false);
        virtual ~FullRenderPass();

        FullRenderPass( const FullRenderPass& ) = delete;
        FullRenderPass& operator=( const FullRenderPass& ) = delete;

        bool init(VkDescriptorPool pool, VkSampler sampler);
        void destroy();

        VkCommandBuffer startRecording(size_t imageIndex, size_t frameIndex, VkSubpassContents contents); ///Consume current dynamic render target
        bool nextRenderTarget(VkSubpassContents contents); ///Consume current dynamic render target
        bool endRecording();

        void addDynamicRenderTarget(VRenderTarget* renderTarget);
        void clearDynamicRenderTargets(); ///Dynamic render targets are already automatically discarded when start recording

        void addWaitSemaphore(size_t frameIndex, VkSemaphore semaphore, VkPipelineStageFlags stage);
        void addSignalSemaphore(size_t frameIndex, VkSemaphore semaphore);

        void setExtent(VkExtent2D extent);
        void setCmbUsage(VkFlags usage);

        void setClearValues(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth);

        void addAttachments(const std::vector<VFramebufferAttachment> &attachments,
                            VkAttachmentStoreOp storeOp, VkAttachmentLoadOp loadOp, bool fromUniform = false);
        void addAttachmentType(VFramebufferAttachmentType type,
                               VkAttachmentStoreOp storeOp, VkAttachmentLoadOp loadOp, bool fromUniform = false);
        void addAttachmentType(VFramebufferAttachmentType type,
                               VkAttachmentStoreOp storeOp, bool toMemory,
                               VkAttachmentLoadOp loadOp, bool fromUniform);
        void addUniforms(const std::vector<VFramebufferAttachment> &attachments);
        void addUniforms(const std::vector<VBuffer> &buffers);
        void addUniforms(const std::vector<VkImageView> &views);

        void setAttachmentsLoadOp(size_t attachmentIndex, VkAttachmentLoadOp loadOp, bool fromUniform);
        void setAttachmentsStoreOp(size_t attachmentIndex, VkAttachmentStoreOp storeOp, bool toUniform = false);

        VkExtent2D          getExtent();
        VRenderPass        *getRenderPass();
        bool                isFinalPass();
        bool                useDynamicRenderTargets();
        bool                needToSubmit();

        const  std::vector<VkPipelineStageFlags> &getWaitSemaphoresStages();
        const  std::vector<VkSemaphore> &getWaitSemaphores(size_t frameIndex);
        const  std::vector<VkSemaphore> &getSignalSemaphores(size_t frameIndex);
        const  std::vector<VFramebufferAttachment> &getAttachments(size_t attachmentIndex);

        const VkCommandBuffer  *getPrimaryCmb(size_t imageIndex, size_t frameIndex);
        VkDescriptorSetLayout   getDescriptorLayout();
        VkDescriptorSet         getDescriptorSet(size_t imageIndex);

    protected:
        bool createCommandBuffers();
        bool createRenderPass();
        bool createRenderTarget();
        bool createDescriptorSetLayout();
        bool createDescriptorSets(VkDescriptorPool pool, VkSampler sampler);

    private:
        size_t      m_imagesCount;
        size_t      m_framesCount;

        bool        m_isFinalPass;
        bool        m_needToSubmit;

        VCommandBuffer  m_cmb;
        VRenderPass     m_renderPass;
        VRenderTarget  *m_renderTarget;

        bool                        m_useDynamicRenderTargets;
        std::list<VRenderTarget*>   m_dynamicRenderTargets;

        std::vector<VkPipelineStageFlags>      m_waitSemaphoreStages;
        std::vector<std::vector<VkSemaphore> > m_waitSemaphores;
        std::vector<std::vector<VkSemaphore> > m_signalSemaphores;

        size_t m_curUniformBinding;
        std::vector<std::pair<size_t, std::vector<VFramebufferAttachment> > >  m_uniformAttachments;
        std::vector<std::pair<size_t, std::vector<VBuffer> > >                 m_uniformBuffers;
        std::vector<std::pair<size_t, std::vector<VkImageView> > >             m_uniformViews;
        VkDescriptorSetLayout           m_descriptorSetLayout;
        std::vector<VkDescriptorSet>    m_descriptorSets;

        size_t m_lastImageIndex;

};

}

#endif // FULLRENDERPASS_H
