#ifndef FULLRENDERPASS_H
#define FULLRENDERPASS_H

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

class FullRenderPass
{
    public:
        FullRenderPass(size_t imagesCount, size_t framesCount);
        virtual ~FullRenderPass();

        bool init();
        void destroy();

        VkCommandBuffer startRecording(size_t imageIndex, size_t frameIndex, VkSubpassContents contents);
        bool endRecording();

        void addWaitSemaphores(const std::vector<VkSemaphore> &semaphores, VkPipelineStageFlags stage);
        void setSignalSemaphores(size_t frameIndex, VkSemaphore *semaphore);

        void setExtent(VkExtent2D extent);
        void setCmbCount(size_t cmbCount);
        void setCmbUsage(VkFlags usage);

        void setClearValues(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth);
        void setAttachments(size_t bufferIndex, const std::vector<VFramebufferAttachment> &attachments);

        VkFlags         getCmbUsage();
        VkExtent2D      getExtent();
        VkRenderPass    getVkRenderPass();
        bool            isFinalPass();

        const  std::vector<VkPipelineStageFlags> &getWaitSemaphoresStages();
        const  std::vector<VkSemaphore> &getWaitSemaphores(size_t frameIndex);
        VkSemaphore *getSignalSemaphore(size_t frameIndex);

        const VkCommandBuffer *getPrimaryCmb(size_t imageIndex, size_t frameIndex);

    protected:
        bool createRenderPass();
        bool createFramebuffers();
        bool createCmb();

    private:
        size_t      m_imagesCount;
        size_t      m_cmbCount;
        VkExtent2D  m_extent;

        bool        m_isFinalPass;
        VkFlags     m_cmbUsage;

        VkRenderPass m_renderPass;
        std::vector<VkClearValue>  m_clearValues;
        std::vector<VkFramebuffer> m_framebuffers;
        std::vector<std::vector<VFramebufferAttachment> > m_attachments;

        std::vector<VkCommandBuffer>    m_primaryCmb;

        std::vector<VkPipelineStageFlags>      m_waitSemaphoreStages;
        std::vector<std::vector<VkSemaphore> > m_waitSemaphores;
        std::vector<VkSemaphore>               m_signalSemaphores;

        size_t m_curRecordingIndex;

};

}

#endif // FULLRENDERPASS_H
