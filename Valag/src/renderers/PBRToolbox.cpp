#include "Valag/renderers/PBRToolbox.h"

#include "Valag/core/VApp.h"

#include <sstream>

namespace vlg
{


const char *PBRToolbox::BRDFLUT_VERTSHADERFILE = "lighting/brdflut.vert.spv";
const char *PBRToolbox::BRDFLUT_FRAGSHADERFILE = "lighting/brdflut.frag.spv";

const int   PBRToolbox::ENVMAP_FILTERINGMIPSCOUNT = 5;

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
    uint32_t width  = 512;
    uint32_t height = 512;

    if(!VulkanHelpers::createAttachment(width,height,VK_FORMAT_R8G8_UNORM,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,m_brdflutAttachement))
        return (false);

    ///Render pass
    VRenderPass renderPass;
    renderPass.addAttachmentType(m_brdflutAttachement.type, VK_ATTACHMENT_STORE_OP_STORE, true,
                                 VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    renderPass.init();

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

    ///Cleaning
    vkDestroyFramebuffer(VInstance::device(), framebuffer, nullptr);
    pipeline.destroy();
    renderPass.destroy();

    return (true);
}

VFramebufferAttachment PBRToolbox::generateFilteredEnvMap(VTexture src)
{
    VFramebufferAttachment dst;

    VulkanHelpers::createAttachment( src.getExtent().width,
                                     src.getExtent().height,
                                     ENVMAP_FILTERINGMIPSCOUNT,
                                     src.getFormat(),
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                     dst);

    return dst;
}


void PBRToolbox::cleanup()
{
    VulkanHelpers::destroyAttachment(m_brdflutAttachement);
}

}
