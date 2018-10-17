#ifndef FULLRENDERPASS_H
#define FULLRENDERPASS_H

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

///Perhaps this should actually inherit from RenderTarget beh (and called renderNode ?)
class FullRenderPass
{
    public:
        FullRenderPass(size_t imagesCount, size_t framesCount);
        virtual ~FullRenderPass();

        bool init(VkDescriptorPool pool, VkSampler sampler);
        void destroy();

        VkCommandBuffer startRecording(size_t imageIndex, size_t frameIndex, VkSubpassContents contents);
        bool endRecording();

        void addWaitSemaphore(size_t frameIndex, VkSemaphore semaphore, VkPipelineStageFlags stage);
        void addSignalSemaphore(size_t frameIndex, VkSemaphore semaphore);

        void setExtent(VkExtent2D extent);
        void setCmbCount(size_t cmbCount);
        void setCmbUsage(VkFlags usage);

        void setClearValues(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth);
        //void setAttachments(size_t bufferIndex, const std::vector<VFramebufferAttachment> &attachments);
        void addAttachments(const std::vector<VFramebufferAttachment> &attachments,
                            VkAttachmentStoreOp storeOp, VkAttachmentLoadOp loadOp, bool fromUniform = false);
        void addUniforms(const std::vector<VFramebufferAttachment> &attachments);
        void addUniforms(const std::vector<VBuffer> &buffers);
        void addUniforms(const std::vector<VkImageView> &views);

        void setAttachmentsLoadOp(size_t attachmentIndex, VkAttachmentLoadOp loadOp, bool fromUniform);
        void setAttachmentsStoreOp(size_t attachmentIndex, VkAttachmentStoreOp storeOp, bool toUniform = false);

        //VkFlags         getCmbUsage();
        VkExtent2D      getExtent();
       // //VkRenderPass    getVkRenderPass();
        const VRenderPass *getRenderPass();
        bool            isFinalPass();
       // size_t          getColorAttachmentsCount();

        const  std::vector<VkPipelineStageFlags> &getWaitSemaphoresStages();
        const  std::vector<VkSemaphore> &getWaitSemaphores(size_t frameIndex);
        //VkSemaphore getSignalSemaphore(size_t frameIndex);
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

        VCommandBuffer  m_cmb;
        VRenderPass     m_renderPass;
        VRenderTarget   m_renderTarget;

        std::vector<VkPipelineStageFlags>      m_waitSemaphoreStages;
        std::vector<std::vector<VkSemaphore> > m_waitSemaphores;
        std::vector<std::vector<VkSemaphore> > m_signalSemaphores;

        size_t m_curUniformBinding;
        std::vector<std::pair<size_t, std::vector<VFramebufferAttachment> > >  m_uniformAttachments;
        std::vector<std::pair<size_t, std::vector<VBuffer> > >                 m_uniformBuffers;
        std::vector<std::pair<size_t, std::vector<VkImageView> > >             m_uniformViews;
        VkDescriptorSetLayout           m_descriptorSetLayout;
        std::vector<VkDescriptorSet>    m_descriptorSets;

};

}

#endif // FULLRENDERPASS_H
