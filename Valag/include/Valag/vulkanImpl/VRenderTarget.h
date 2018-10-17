#ifndef VRENDERTARGET_H
#define VRENDERTARGET_H

#include "Valag/vulkanImpl/VRenderPass.h"

namespace vlg
{

class VRenderTarget
{
    public:
        VRenderTarget();
        virtual ~VRenderTarget();

        bool init(size_t framebuffersCount, VRenderPass *renderPass/*, size_t cmbCount*/);
        void destroy();

        ///Add preexisting attachments
        void addAttachments(const std::vector<VFramebufferAttachment> &attachments);
        ///Create new attachments (that will be created when init)
        void createAttachments(VFramebufferAttachmentType type);

        void startRendering(size_t framebufferIndex, VkCommandBuffer cmb, VkSubpassContents contents);
        void startRendering(size_t framebufferIndex, VkCommandBuffer cmb, VkSubpassContents contents,
                            VRenderPass* renderPass);
        //VkCommandBuffer startRecording(size_t cmbIndex, size_t framebufferIndex, VkSubpassContents contents);
        //bool endRecording();

        void setExtent(VkExtent2D extent);
        void setClearValue(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth = {0.0,0});
        //void setCmbUsage(VkFlags usage);

        VkExtent2D  getExtent();
        //VkFlags     getCmbUsage();
        //const VkCommandBuffer *getPrimaryCmb(size_t cmbIndex);
        const  std::vector<VFramebufferAttachment> &getAttachments(size_t attachmentIndex);

    protected:
        bool createFramebuffers(size_t framebuffersCount);
        //bool createCmb(size_t cmbCount);

    protected:
        size_t      m_imagesCount;
        VkExtent2D  m_extent;
        //VkFlags     m_cmbUsage;

        std::vector<VkClearValue>   m_clearValues;
        std::vector<VkFramebuffer>  m_framebuffers;

        VRenderPass *m_defaultRenderPass;
        std::vector<std::vector<VFramebufferAttachment> > m_attachments;

        //std::vector<VkCommandBuffer> m_primaryCmb;

        //size_t m_curRecordingIndex;
};

}

#endif // VRENDERTARGET_H
