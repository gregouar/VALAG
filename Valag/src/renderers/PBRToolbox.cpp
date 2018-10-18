#include "Valag/renderers/PBRToolbox.h"

#include "Valag/core/VApp.h"

#include <sstream>

namespace vlg
{


const char *PBRToolbox::BRDFLUT_VERTSHADERFILE = "lighting/brdflut.vert.spv";
const char *PBRToolbox::BRDFLUT_FRAGSHADERFILE = "lighting/brdflut.frag.spv";
const char *PBRToolbox::IBLFILTERING_VERTSHADERFILE = "lighting/iblfiltering.vert.spv";
const char *PBRToolbox::IBLFILTERING_FRAGSHADERFILE = "lighting/iblfiltering.frag.spv";

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

    ///Rendertarget
    VRenderTarget renderTarget;
    renderTarget.addAttachments({m_brdflutAttachement});
    renderTarget.init(1, &renderPass);

    ///Framebuffer
    /*VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = renderPass.getVkRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments    = &m_brdflutAttachement.view;
    framebufferInfo.width           = width;
    framebufferInfo.height          = height;
    framebufferInfo.layers          = 1;

    VkFramebuffer framebuffer;
    if(vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
        return (false);*/

   /* VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass   = renderPass.getVkRenderPass();
    renderPassBeginInfo.framebuffer  = framebuffer;
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = {width, height};*/

    VkCommandBuffer cmb = VInstance::beginSingleTimeCommands();

    renderTarget.startRendering(0, cmb);

    //vkCmdBeginRenderPass(cmb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        pipeline.bind(cmb);
        vkCmdDraw(cmb, 3, 1, 0, 0);
    vkCmdEndRenderPass(cmb);

    VInstance::endSingleTimeCommands(cmb);

    ///Cleaning
    //vkDestroyFramebuffer(VInstance::device(), framebuffer, nullptr);
    pipeline.destroy();
    renderTarget.destroy();
    renderPass.destroy();

    return (true);
}

VFramebufferAttachment PBRToolbox::generateFilteredEnvMap(VTexture src)
{
    VFramebufferAttachment dst;

    uint32_t minExtent = std::pow(2, PBRToolbox::ENVMAP_FILTERINGMIPSCOUNT);
    uint32_t width  = std::max(src.getExtent().width, minExtent);
    uint32_t height = std::max(src.getExtent().height, minExtent);

    VulkanHelpers::createAttachment( width,
                                     height,
                                     ENVMAP_FILTERINGMIPSCOUNT,
                                     src.getFormat(),
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                     dst);

    ///Render pass
    VRenderPass renderPass;
    renderPass.addAttachmentType(dst.type, VK_ATTACHMENT_STORE_OP_STORE, true,
                                 VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    renderPass.init();

     ///Pipeline
    VGraphicsPipeline pipeline;

    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << IBLFILTERING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << IBLFILTERING_FRAGSHADERFILE;

    pipeline.addSpecializationDatum(ENVMAP_FILTERINGMIPSCOUNT, 1);

    pipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    pipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    struct {
        float   roughness;
        int     srcId;
        int     srcLayer;
    } pc;

    pc.srcId    = src.getTextureId();
    pc.srcLayer = src.getTextureLayer();

    pipeline.attachDescriptorSetLayout(VTexturesManager::descriptorSetLayout());
    pipeline.attachPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(pc));

    if(!pipeline.init(&renderPass))
    {/* error */}

    ///RenderTarget
    VRenderTarget renderTargets[ENVMAP_FILTERINGMIPSCOUNT];
    for(size_t i = 0 ; i < ENVMAP_FILTERINGMIPSCOUNT ; ++i)
    {
        renderTargets[i].addAttachments({dst});
        renderTargets[i].setMipLevel(i);
        renderTargets[i].init(1, &renderPass);
    }

    ///Framebuffer
    /*VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = renderPass.getVkRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.layers          = 1;

    VkFramebuffer framebuffers[ENVMAP_FILTERINGMIPSCOUNT];
    for(size_t i = 0 ; i < ENVMAP_FILTERINGMIPSCOUNT ; ++i)
    {
        framebufferInfo.pAttachments = &dst.mipViews[i];
        framebufferInfo.width        = dst.extent.width  * std::pow(0.5, i);
        framebufferInfo.height       = dst.extent.height * std::pow(0.5, i);
        if(vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
        {}
    }*/

    ///Rendering
    /*VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass   = renderPass.getVkRenderPass();
    renderPassBeginInfo.renderArea.offset = {0, 0};*/

   /* VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};*/

    VkCommandBuffer cmb = VInstance::beginSingleTimeCommands();

    pipeline.bind(cmb);

    for(size_t i = 0 ; i < ENVMAP_FILTERINGMIPSCOUNT ; ++i)
    {
        renderTargets[i].startRendering(0, cmb, VK_SUBPASS_CONTENTS_INLINE);

        /*viewport.width  = static_cast<float>(renderTargets[i].getExtent().width);
        viewport.height = static_cast<float>(renderTargets[i].getExtent().height);
        scissor.extent = renderTargets[i].getExtent();*/

        /*renderPassBeginInfo.renderArea.extent = {static_cast<uint32_t>((double)dst.extent.width  * std::pow(0.5, i)),
                                                 static_cast<uint32_t>((double)dst.extent.height * std::pow(0.5, i))};

        viewport.width  = static_cast<float>(renderPassBeginInfo.renderArea.extent.width);
        viewport.height = static_cast<float>(renderPassBeginInfo.renderArea.extent.height);

        scissor.extent = renderPassBeginInfo.renderArea.extent;

        renderPassBeginInfo.framebuffer = framebuffers[i];
        vkCmdBeginRenderPass(cmb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);*/

            pipeline.updateViewport(cmb, {0,0}, renderTargets[i].getExtent());

            //vkCmdSetViewport(cmb, 0, 1, &viewport);
            //vkCmdSetScissor(cmb, 0, 1, &scissor);

            VkDescriptorSet descSets[] = {VTexturesManager::descriptorSet()};

            vkCmdBindDescriptorSets(cmb,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline.getLayout(),0,1, descSets, 0, nullptr);

            pc.roughness = static_cast<float>(i)/static_cast<float>(ENVMAP_FILTERINGMIPSCOUNT);

            vkCmdPushConstants(cmb, pipeline.getLayout(),
                    VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), (void*)&pc);

            vkCmdDraw(cmb, 3, 1, 0, 0);
        vkCmdEndRenderPass(cmb);
    }

    VInstance::endSingleTimeCommands(cmb);

    ///Cleaning
    for(size_t i = 0 ; i < ENVMAP_FILTERINGMIPSCOUNT ; ++i)
        renderTargets[i].destroy();
        //vkDestroyFramebuffer(VInstance::device(), framebuffers[i], nullptr);

    pipeline.destroy();
    renderPass.destroy();


    return dst;
}


void PBRToolbox::cleanup()
{
    VulkanHelpers::destroyAttachment(m_brdflutAttachement);
}

}
