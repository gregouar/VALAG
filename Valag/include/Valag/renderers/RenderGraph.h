#ifndef RENDERGRAPH_H
#define RENDERGRAPH_H

#include <set>

#include "Valag/renderers/FullRenderPass.h"

namespace vlg
{

///Maybe I should drop working with indices and directly return FullRenderPass*

class RenderGraph
{
    public:
        RenderGraph(size_t imagesCount, size_t framesCount);
        virtual ~RenderGraph();

        bool init();
        void destroy();

        void    setDefaultExtent(VkExtent2D extent);

        ///If VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT is set, then the cmb will use framesCount and frameIndex
        ///Otherwise it will use imagesCount and imageIndex
        size_t  addRenderPass(VkFlags usage = 0);
        size_t  addDynamicRenderPass();

        ///Only use if you need explicit synchronization with semaphores. For most cases, the barriers will suffice
        void    connectRenderPasses(size_t src, size_t dst);

        void    transferAttachmentsToAttachments(size_t srcRenderPass, size_t dstRenderPass, size_t attachmentsIndex,
                                    VkAttachmentStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);
        void    transferAttachmentsToUniforms(size_t srcRenderPass, size_t dstRenderPass, size_t attachmentsIndex);

        ///If an attachment is of type VK_IMAGE_LAYOUT_PRESENT_SRC_KHR then the cmb will be returned by submitToGraphicsQueue in order to be
        ///rendered by RenderWindow
        void    addAttachmentType(size_t renderPassIndex, const VFramebufferAttachmentType &type,
                               VkAttachmentStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, VkAttachmentLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        void    addAttachmentType(size_t renderPassIndex, const VFramebufferAttachmentType &type,
                                    VkAttachmentStoreOp storeOp, bool toMemory,
                                    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, bool fromMemory = false);
        void    addNewAttachments(size_t renderPassIndex, const VFramebufferAttachment &attachment,
                               VkAttachmentStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, VkAttachmentLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        void    addNewAttachments(size_t renderPassIndex, const std::vector<VFramebufferAttachment> &attachments,
                               VkAttachmentStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, VkAttachmentLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        void    addNewUniforms(size_t renderPassIndex, const std::vector<VBuffer> &buffers);
        void    addNewUniforms(size_t renderPassIndex, const std::vector<VkImageView> &views);

        void    setClearValue(size_t renderPassIndex, size_t attachmentIndex ,glm::vec4 color, glm::vec2 depth);

        VRenderPass            *getRenderPass(size_t renderPassIndex);
        VkDescriptorSetLayout   getDescriptorLayout(size_t renderPassIndex);
        VkDescriptorSet         getDescriptorSet(size_t renderPassIndex, size_t imageIndex);

        VkExtent2D  getExtent(size_t renderPassIndex);

        void addDynamicRenderTarget(size_t renderPassIndex, VRenderTarget* renderTarget);

        VkCommandBuffer startRecording(size_t renderPassIndex, size_t imageIndex, size_t frameIndex,
                                       VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
        bool            nextRenderTarget(size_t renderPassIndex, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
        bool            endRecording(size_t renderPassIndex);

        ///Return final render passes
        std::vector<FullRenderPass*> submitToGraphicsQueue(size_t imageIndex, size_t frameIndex);

    protected:
        bool createSemaphores();
        bool createDescriptorPool();
        bool createSampler();
        bool initRenderPasses();

    protected:
        VkExtent2D  m_defaultExtent;
        size_t      m_imagesCount, m_framesCount;

        std::set<std::pair<size_t, size_t> > m_connexions; //Second one is waiting for first one

        std::vector<FullRenderPass*> m_renderPasses;
        std::list<VkSemaphore>       m_semaphores;

        std::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
        VkDescriptorPool    m_descriptorPool;
        VkSampler           m_sampler;

    private:

};

}

#endif // RENDERGRAPH_H
