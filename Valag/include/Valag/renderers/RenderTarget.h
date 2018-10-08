#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

///Maybe I should rename it in VRenderPass

class RenderTarget
{
    public:
        RenderTarget();
        virtual ~RenderTarget();

        bool init(size_t framebuffersCount, size_t cmbCount, VRenderPass *renderPass);
        void destroy();

        ///Add preexisting attachments
        void addAttachments(const std::vector<VFramebufferAttachment> &attachments);
        ///Create new attachments (that will be created when init)
        void createAttachments(VFramebufferAttachmentType type);

        VkCommandBuffer startRecording(size_t cmbIndex, size_t framebufferIndex, VkSubpassContents contents);
        bool endRecording();

        void setExtent(VkExtent2D extent);
        void setClearValue(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth = {0.0,0});
        void setCmbUsage(VkFlags usage);

        VkExtent2D  getExtent();
        VkFlags     getCmbUsage();
        const VkCommandBuffer *getPrimaryCmb(size_t cmbIndex);
        const  std::vector<VFramebufferAttachment> &getAttachments(size_t attachmentIndex);

    protected:
        bool createFramebuffers(size_t framebuffersCount);
        bool createCmb(size_t cmbCount);

    protected:
        size_t      m_imagesCount;
        VkExtent2D  m_extent;
        VkFlags     m_cmbUsage;

        std::vector<VkClearValue>   m_clearValues;
        std::vector<VkFramebuffer>  m_framebuffers;

        VRenderPass *m_usedRenderPass;
        std::vector<std::vector<VFramebufferAttachment> > m_attachments;

        std::vector<VkCommandBuffer> m_primaryCmb;

        size_t m_curRecordingIndex;

};

}

#endif // RENDERTARGET_H
