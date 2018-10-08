#include "Valag/vulkanImpl/VRenderPass.h"

#include "Valag/utils/Logger.h"
#include "Valag/vulkanImpl/VInstance.h"

namespace vlg
{

VRenderPass::VRenderPass() :
    m_vkRenderPass(VK_NULL_HANDLE)
{
    //ctor
}

VRenderPass::~VRenderPass()
{
    this->destroy();
}

bool VRenderPass::init()
{
    if(!this->createRenderPass())
    {
        Logger::error("Cannot create render pass");
        return (false);
    }

    return (true);
}

void VRenderPass::destroy()
{
    VkDevice device = VInstance::device();

    if(m_vkRenderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(device, m_vkRenderPass, nullptr);
    m_vkRenderPass = VK_NULL_HANDLE;

    m_attachmentsType.clear();
}

void VRenderPass::addAttachmentType(const VFramebufferAttachmentType &type,
                                    VkAttachmentStoreOp storeOp, bool toMemory,
                                    VkAttachmentLoadOp loadOp, bool fromMemory)
{
    m_attachmentsType.push_back(type);
    m_attachmentsLoadOp.push_back({loadOp, fromMemory});
    m_attachmentsStoreOp.push_back({storeOp, toMemory});
}


void VRenderPass::setAttachmentsLoadOp(size_t attachmentIndex, VkAttachmentLoadOp loadOp, bool fromMemory)
{
    m_attachmentsLoadOp[attachmentIndex] = {loadOp, fromMemory};
}

void VRenderPass::setAttachmentsStoreOp(size_t attachmentIndex, VkAttachmentStoreOp storeOp, bool toMemory)
{
    m_attachmentsStoreOp[attachmentIndex] = {storeOp, toMemory};
}

VkRenderPass VRenderPass::getVkRenderPass()
{
    return m_vkRenderPass;
}

size_t VRenderPass::getColorAttachmentsCount()
{
    size_t c = 0;

    for(auto &type : m_attachmentsType)
        if(type.layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        || type.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            c++;

    return c;
}

/// Protected ///


bool VRenderPass::createRenderPass()
{
    std::vector<VkAttachmentDescription> attachments(m_attachmentsType.size(), VkAttachmentDescription{});

    std::vector<VkAttachmentReference> colorAttachmentRef;

    bool hasDepthAttachment = false;
    VkAttachmentReference depthAttachmentRef{};

    for(size_t i = 0 ; i < attachments.size() ; ++i)
    {
        VkImageLayout layout = m_attachmentsType[i].layout;

        attachments[i].format = m_attachmentsType[i].format;

        attachments[i].samples          = VK_SAMPLE_COUNT_1_BIT;
        attachments[i].loadOp           = m_attachmentsLoadOp[i].first;
        attachments[i].storeOp          = m_attachmentsStoreOp[i].first;//VK_ATTACHMENT_STORE_OP_STORE;

        ///Maybe I should check if existence of stencil buffer first
        attachments[i].stencilLoadOp    = m_attachmentsLoadOp[i].first;///VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[i].stencilStoreOp   = m_attachmentsStoreOp[i].first;///VK_ATTACHMENT_STORE_OP_DONT_CARE;

        if(m_attachmentsLoadOp[i].first == VK_ATTACHMENT_LOAD_OP_LOAD)
        {
            if(m_attachmentsLoadOp[i].second)
                attachments[i].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            else
                attachments[i].initialLayout = layout;
        }
        else
            attachments[i].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;

        if(layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            attachments[i].finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            ///m_isFinalPass = true;
        }
        else if(m_attachmentsStoreOp[i].first == VK_ATTACHMENT_STORE_OP_STORE && !m_attachmentsStoreOp[i].second)
             attachments[i].finalLayout     = layout;
        else
            attachments[i].finalLayout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if(layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            colorAttachmentRef.push_back({static_cast<uint32_t>(i),
                                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        if(layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            depthAttachmentRef = {static_cast<uint32_t>(i),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
            hasDepthAttachment = true;
        }
    }

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRef.size());
    subpassDescription.pColorAttachments = colorAttachmentRef.data();

    if(hasDepthAttachment)
        subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkSubpassDependency, 4> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = /*VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |*/ VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[2].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[2].dstSubpass = 0;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = /*VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |*/ VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[3].srcSubpass = 0;
    dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[3].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[3].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[3].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[3].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount  = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments     = attachments.data();
    renderPassInfo.subpassCount     = 1;
    renderPassInfo.pSubpasses       = &subpassDescription;
    renderPassInfo.dependencyCount  = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies    = dependencies.data();

    return (vkCreateRenderPass(VInstance::device(), &renderPassInfo, nullptr, &m_vkRenderPass) == VK_SUCCESS);
}

}
