#include "Valag/vulkanImpl/VRenderTarget.h"

#include "Valag/vulkanImpl/VulkanHelpers.h"
#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

VRenderTarget::VRenderTarget() :
    m_imagesCount(1),
    m_extent({0,0}),
    m_mipLevel(0),
 //   m_cmbUsage(0),
    m_defaultRenderPass(nullptr)
   // m_curRecordingIndex(0)
{
    //ctor
}

VRenderTarget::~VRenderTarget()
{
    this->destroy();
}

bool VRenderTarget::init(size_t framebuffersCount, VRenderPass *renderPass)
{
    m_defaultRenderPass = renderPass;

    if(!this->createFramebuffers(framebuffersCount))
        return (false);

    return (true);
}

void VRenderTarget::destroy()
{
    VkDevice device = VInstance::device();

    m_clearValues.clear();
    for(auto framebuffer : m_framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    m_framebuffers.clear();
}

void VRenderTarget::addAttachments(const std::vector<VFramebufferAttachment> &attachments)
{
    m_attachments.push_back(attachments);
}

void VRenderTarget::createAttachments(VFramebufferAttachmentType type)
{
    /** do something **/
}


void VRenderTarget::startRendering(size_t framebufferIndex, VkCommandBuffer cmb, VkSubpassContents contents)
{
    this->startRendering(framebufferIndex, cmb, contents, m_defaultRenderPass);
}

void VRenderTarget::startRendering(size_t framebufferIndex, VkCommandBuffer cmb, VkSubpassContents contents,
                                   VRenderPass* renderPass)
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass   = renderPass->getVkRenderPass();
    renderPassInfo.framebuffer  = m_framebuffers[framebufferIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_extent;

    renderPassInfo.clearValueCount  = static_cast<uint32_t>(m_clearValues.size());
    renderPassInfo.pClearValues     = m_clearValues.data();

    vkCmdBeginRenderPass(cmb, &renderPassInfo, contents);
}

void VRenderTarget::setExtent(VkExtent2D extent)
{
    m_extent = extent;
}

void VRenderTarget::setMipLevel(size_t mipLevel)
{
    m_mipLevel = mipLevel;
}

void VRenderTarget::setClearValue(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth)
{
    if(attachmentIndex >= m_clearValues.size())
        m_clearValues.resize(attachmentIndex+1, VkClearValue{});

    m_clearValues[attachmentIndex].color        = {color.r, color.g, color.b, color.a};
    m_clearValues[attachmentIndex].depthStencil = {depth.x, static_cast<uint32_t>(depth.y)};
}

VkExtent2D VRenderTarget::getExtent()
{
    return m_extent;
}

const  std::vector<VFramebufferAttachment> &VRenderTarget::getAttachments(size_t attachmentsIndex)
{
    if(attachmentsIndex >= m_attachments.size())
        throw std::runtime_error("Cannot get attachment");
    return m_attachments[attachmentsIndex];
}


/// Protected ///

bool VRenderTarget::createFramebuffers(size_t framebuffersCount)
{
    m_framebuffers.resize(framebuffersCount);

    float mipFactor = std::pow(0.5, static_cast<float>(m_mipLevel));

    for (size_t i = 0; i < framebuffersCount ; ++i)
    {
        std::vector<VkImageView> attachments(m_attachments.size());

        for(size_t j = 0 ; j < attachments.size() ; ++j)
        {
            if(m_extent.width  != mipFactor*m_attachments[j][i].extent.width
            || m_extent.height != mipFactor*m_attachments[j][i].extent.height)
            {
                m_extent.width  = mipFactor*m_attachments[j][i].extent.width;
                m_extent.height = mipFactor*m_attachments[j][i].extent.height;
            }
            attachments[j] = m_attachments[j][i].mipViews[m_mipLevel];
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = m_defaultRenderPass->getVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = m_extent.width;
        framebufferInfo.height          = m_extent.height;
        framebufferInfo.layers          = 1;

        if (vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
        {
            Logger::error("Cannot create framebuffers");
            m_framebuffers.clear();
            return (false);
        }
    }

    if(m_clearValues.size() < m_attachments.size())
        m_clearValues.resize(m_attachments.size(), VkClearValue{});

    return (true);
}

}
