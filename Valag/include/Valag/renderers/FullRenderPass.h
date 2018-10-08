#ifndef FULLRENDERPASS_H
#define FULLRENDERPASS_H

#include "Valag/vulkanImpl/vulkanImpl.h"
#include "Valag/renderers/RenderTarget.h"

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

        //void addWaitSemaphores(const std::vector<VkSemaphore> &semaphores, VkPipelineStageFlags stage);
       // void setSignalSemaphores(size_t frameIndex, VkSemaphore semaphore);
        //void addSignalSemaphores(const std::vector<VkSemaphore> &semaphores);

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

        VkFlags         getCmbUsage();
        VkExtent2D      getExtent();
        VkRenderPass    getVkRenderPass();
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
        bool createRenderPass();
        /*bool createFramebuffers();
        bool createCmb();*/
        bool createRenderTarget();
        bool createDescriptorSetLayout();
        bool createDescriptorSets(VkDescriptorPool pool, VkSampler sampler);

    private:
        size_t      m_imagesCount;
        size_t      m_cmbCount;
       // VkExtent2D  m_extent;

        bool        m_isFinalPass;
        //VkFlags     m_cmbUsage;

        VRenderPass     m_renderPass;
        RenderTarget    m_renderTarget;
        //VkRenderPass m_vkRenderPass;
        //std::vector<VkClearValue>  m_clearValues;
        //std::vector<VkFramebuffer> m_framebuffers;

        //std::vector<std::vector<VFramebufferAttachment> > m_attachments;
        //std::vector<std::pair<VkAttachmentLoadOp, bool> > m_attachmentsLoadOp; //loadOp and fromUniform
        //std::vector<std::pair<VkAttachmentStoreOp, bool> > m_attachmentsStoreOp; //storeOp and toUniform

        //std::vector<VkCommandBuffer>    m_primaryCmb;

        std::vector<VkPipelineStageFlags>      m_waitSemaphoreStages;
        std::vector<std::vector<VkSemaphore> > m_waitSemaphores;
        std::vector<std::vector<VkSemaphore> > m_signalSemaphores;

        size_t m_curUniformBinding;
        std::vector<std::pair<size_t, std::vector<VFramebufferAttachment> > >  m_uniformAttachments;
        std::vector<std::pair<size_t, std::vector<VBuffer> > >                 m_uniformBuffers;
        std::vector<std::pair<size_t, std::vector<VkImageView> > >             m_uniformViews;
        VkDescriptorSetLayout           m_descriptorSetLayout;
        std::vector<VkDescriptorSet>    m_descriptorSets;

        //size_t m_curRecordingIndex;

};

}

#endif // FULLRENDERPASS_H
