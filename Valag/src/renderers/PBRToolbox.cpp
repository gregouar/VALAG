#include "Valag/renderers/PBRToolbox.h"

#include "Valag/core/VApp.h"

#include <sstream>

namespace vlg
{


const char *PBRToolbox::BRDFLUT_VERTSHADERFILE = "lighting/brdflut.vert.spv";
const char *PBRToolbox::BRDFLUT_FRAGSHADERFILE = "lighting/brdflut.frag.spv";

PBRToolbox::PBRToolbox()
{
    m_brdflutAttachement.image.vkImage = VK_NULL_HANDLE;
    m_brdflutAttachement.view = VK_NULL_HANDLE;
}

PBRToolbox::~PBRToolbox()
{
    this->cleanup();
}

VFramebufferAttachment PBRToolbox::getBrdflut()
{
    if(PBRToolbox::instance()->m_brdflutAttachement.image.vkImage == VK_NULL_HANDLE)
        PBRToolbox::instance()->generateBrdflut();
    return PBRToolbox::instance()->m_brdflutAttachement;
}

///I could turn part of this into some makeSingleUseRenderPass();
bool PBRToolbox::generateBrdflut()
{
    /*VulkanHelpers::createImage(512,512,1,VK_FORMAT_R8G8_UNORM,VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_brdflutImage);*/
    uint32_t width  = 512;
    uint32_t height = 512;

    if(!VulkanHelpers::createAttachment(width,height,VK_FORMAT_R8G8_UNORM,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,m_brdflutAttachement))
        return (false);

    ///Why am I not using a full render pass ?

    ///Render pass
    VRenderPass renderPass;
    renderPass.addAttachmentType(m_brdflutAttachement.type, VK_ATTACHMENT_STORE_OP_STORE, true,
                                 VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    renderPass.init();

    ///Render pass
    /*VkAttachmentDescription attachmentDesc = {};
    attachmentDesc.format  = m_brdflutAttachement.type.format;
    attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDesc.loadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDesc.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDesc.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDesc.finalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorRef = {0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorRef;

    VkSubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask =  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask =  VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount  = 1;
    renderPassInfo.pAttachments     = &attachmentDesc;
    renderPassInfo.subpassCount     = 1;
    renderPassInfo.pSubpasses       = &subpassDescription;
    renderPassInfo.dependencyCount  = 2;
    renderPassInfo.pDependencies    = dependencies;

    VkRenderPass pass;
    if(vkCreateRenderPass(VInstance::device(), &renderPassInfo, nullptr, &pass) != VK_SUCCESS)
        return (false);*/

    ///Pipeline
    VGraphicsPipeline pipeline;

    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << BRDFLUT_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << BRDFLUT_FRAGSHADERFILE;

    pipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    pipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    pipeline.setStaticExtent({width,height});

    if(!pipeline.init(&renderPass))
        return (false);

    ///Framebuffer
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = renderPass.getVkRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments    = &m_brdflutAttachement.view;
    framebufferInfo.width           = width;
    framebufferInfo.height          = height;
    framebufferInfo.layers          = 1;

    VkFramebuffer framebuffer;
    if(vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
        return (false);

   /* ///Command buffer
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool   = VInstance::commandPool(COMMANDPOOL_SHORTLIVED);
    allocInfo.level         = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmb;
    if(vkAllocateCommandBuffers(VInstance::device(), &allocInfo, &cmb) != VK_SUCCESS)
        return (false);

    ///Recording
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(cmb, &beginInfo) != VK_SUCCESS)
        return (false);*/

    ///Mmmm if I use RenderTarget, then I would lose this (recycling cmb) => but not really important since one use only I guess
    VkCommandBuffer cmb = VInstance::beginSingleTimeCommands();

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass   = renderPass.getVkRenderPass();
    renderPassBeginInfo.framebuffer  = framebuffer;
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = {width, height};

    vkCmdBeginRenderPass(cmb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        pipeline.bind(cmb);
        vkCmdDraw(cmb, 3, 1, 0, 0);
    vkCmdEndRenderPass(cmb);

    VInstance::endSingleTimeCommands(cmb);

    /*if (vkEndCommandBuffer(cmb) != VK_SUCCESS)
        return (false);*/


    ///Cleaning
    vkDestroyFramebuffer(VInstance::device(), framebuffer, nullptr);
    pipeline.destroy();
    renderPass.destroy();
    //vkDestroyRenderPass(VInstance::device(), pass, nullptr);

    return (true);
}

void PBRToolbox::cleanup()
{
    VulkanHelpers::destroyAttachment(m_brdflutAttachement);
}

}
