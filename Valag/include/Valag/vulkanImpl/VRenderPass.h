#ifndef VRENDERPASS_H
#define VRENDERPASS_H

#include <Vulkan/vulkan.h>
#include <vector>

#include "Valag/vulkanImpl/VulkanHelpers.h"


namespace vlg
{

class VRenderPass
{
    public:
        VRenderPass();
        virtual ~VRenderPass();

        VRenderPass( const VRenderPass& ) = delete;
        VRenderPass& operator=( const VRenderPass& ) = delete;

        bool init();
        void destroy();

        void addAttachmentType(const VFramebufferAttachmentType &type,
                               VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE, bool toMemory = false,
                               VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, bool fromMemory =  false);

        void setAttachmentsLoadOp(size_t attachmentIndex, VkAttachmentLoadOp loadOp, bool fromMemory = false);
        void setAttachmentsStoreOp(size_t attachmentIndex, VkAttachmentStoreOp storeOp, bool toMemory = false);

        //I could do something clever to keep encapsulating
        VkRenderPass    getVkRenderPass() const;
        size_t          getColorAttachmentsCount() const;

    protected:
        bool createRenderPass();

    protected:
        VkRenderPass m_vkRenderPass;
        std::vector<VFramebufferAttachmentType> m_attachmentsType;
        std::vector<std::pair<VkAttachmentLoadOp, bool> > m_attachmentsLoadOp; //loadOp and fromMemory
        std::vector<std::pair<VkAttachmentStoreOp, bool> > m_attachmentsStoreOp; //storeOp and toMemory
};

}

#endif // VRENDERPASS_H
