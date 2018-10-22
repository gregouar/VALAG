#include "Valag/vulkanImpl/VGraphicsPipeline.h"

#include "Valag/vulkanImpl/VInstance.h"
#include "Valag/vulkanImpl/VulkanHelpers.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

VGraphicsPipeline::VGraphicsPipeline() :
    m_pipelineLayout(VK_NULL_HANDLE),
    m_pipeline(VK_NULL_HANDLE)
{
    m_vertexInput = {};
    m_vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vertexInput.vertexBindingDescriptionCount = 0;
    m_vertexInput.vertexAttributeDescriptionCount = 0;

    m_inputAssembly = {};
    m_inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_inputAssembly.primitiveRestartEnable = VK_FALSE;

    m_staticExtent.width = 0.0;
    m_staticExtent.height = 0.0;
    m_onlyScissorIsStatic = false;

    m_cullMode = VK_CULL_MODE_NONE;

    m_depthStencil = {};
    m_depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_depthStencil.depthTestEnable = VK_FALSE;
    m_depthStencil.depthWriteEnable = VK_FALSE;
    m_depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;
    m_depthStencil.depthBoundsTestEnable = VK_FALSE;
    m_depthStencil.minDepthBounds = 0.0f; // Optional
    m_depthStencil.maxDepthBounds = 1.0f; // Optional
    m_depthStencil.stencilTestEnable = VK_FALSE;
    m_depthStencil.front = {}; // Optional
    m_depthStencil.back = {}; // Optional
}

VGraphicsPipeline::~VGraphicsPipeline()
{
    this->destroy();
}


void VGraphicsPipeline::createShader(const std::string &shaderPath, VkShaderStageFlagBits shaderStageBit)
{
    m_attachedShaders.push_back({shaderPath, shaderStageBit});
}

void VGraphicsPipeline::setSpecializationInfo(VkSpecializationInfo &specializationInfo, size_t shaderNbr)
{
    //if(m_specializationInfos.size() < shaderNbr)
      //  m_specializationInfos.resize(shaderNbr+1, {});
    m_specializationInfos.insert(m_specializationInfos.end(), shaderNbr + 1 - m_specializationInfos.size(), VkSpecializationInfo{});
    m_specializationInfos[shaderNbr] = specializationInfo;
}

void VGraphicsPipeline::addSpecializationDatum(float value, size_t shaderNbr)
{
    this->addSpecializationDatum(sizeof(float), (char*)&value, shaderNbr);
}

void VGraphicsPipeline::addSpecializationDatum(bool value, size_t shaderNbr)
{
    this->addSpecializationDatum(sizeof(bool), (char*)&value, shaderNbr);
}

void VGraphicsPipeline::addSpecializationDatum(int value, size_t shaderNbr)
{
    this->addSpecializationDatum(sizeof(int), (char*)&value, shaderNbr);
}

void VGraphicsPipeline::addSpecializationDatum(uint32_t value, size_t shaderNbr)
{
    this->addSpecializationDatum(sizeof(uint32_t), (char*)&value, shaderNbr);
}

void VGraphicsPipeline::addSpecializationDatum(size_t size, char* data, size_t shaderNbr)
{
    if(m_specializationData.size() < shaderNbr)
        m_specializationData.resize(shaderNbr+1);

    if(m_specializationMapEntries.size() < shaderNbr)
        m_specializationMapEntries.resize(shaderNbr+1);

    VkSpecializationMapEntry mapEntry{};
    mapEntry.constantID = m_specializationMapEntries[shaderNbr].size();
    mapEntry.size       = size;
    if(m_specializationMapEntries[shaderNbr].size() > 0)
        mapEntry.offset = m_specializationMapEntries[shaderNbr].back().offset
                            + m_specializationMapEntries[shaderNbr].back().size;

    char *d = new char[size];
    for(size_t i = 0 ; i < size ; ++i)
        *(d+i) = *(data+i);
    m_specializationData[shaderNbr].push_back(d);
    m_specializationMapEntries[shaderNbr].push_back(mapEntry);
}

void VGraphicsPipeline::setVertexInput(size_t vertexBindingCount, VkVertexInputBindingDescription* vertexBindings,
                                        size_t vertexAttributeCount, VkVertexInputAttributeDescription* vertexAttributes)
{
    m_vertexInput.vertexBindingDescriptionCount     = static_cast<uint32_t>(vertexBindingCount);
    m_vertexInput.pVertexBindingDescriptions        = vertexBindings;
    m_vertexInput.vertexAttributeDescriptionCount   = static_cast<uint32_t>(vertexAttributeCount);
    m_vertexInput.pVertexAttributeDescriptions      = vertexAttributes;
}

void VGraphicsPipeline::setInputAssembly(VkPrimitiveTopology topology, bool restart)
{
    m_inputAssembly.topology = topology;
    m_inputAssembly.primitiveRestartEnable = restart ? VK_TRUE : VK_FALSE;// static_cast<VkBool32>(restart);
}

void VGraphicsPipeline::setStaticExtent(VkExtent2D extent, bool onlyScissor)
{
    m_staticExtent = extent;
    m_onlyScissorIsStatic = onlyScissor;
}

void VGraphicsPipeline::setCullMode(VkCullModeFlagBits cullMode)
{
    m_cullMode = cullMode;
}

void VGraphicsPipeline::setBlendMode(BlendMode blendMode, size_t attachmentNbr)
{
    m_blendModes.insert(m_blendModes.end(), attachmentNbr + 1 - m_blendModes.size(), BlendMode_None);
    m_blendModes[attachmentNbr] = blendMode;
}

void VGraphicsPipeline::setWriteMask(VkFlags  writeMask, size_t attachmentNbr)
{
    VkFlags  defaultWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    m_writeMasks.insert(m_writeMasks.end(), attachmentNbr + 1 - m_writeMasks.size(),defaultWriteMask);
    m_writeMasks[attachmentNbr] = writeMask;
}

void VGraphicsPipeline::setCustomBlendMode(VkPipelineColorBlendStateCreateInfo blendMode, size_t attachmentNbr)
{
    m_blendModes.insert(m_blendModes.end(), attachmentNbr + 1 - m_blendModes.size(), BlendMode_None);
    m_customBlends.insert(m_customBlends.end(), attachmentNbr + 1 - m_customBlends.size(), VkPipelineColorBlendStateCreateInfo{});

    m_blendModes[attachmentNbr]     = BlendMode_Custom;
    m_customBlends[attachmentNbr]   = blendMode;
}

void VGraphicsPipeline::attachDescriptorSetLayout(VkDescriptorSetLayout setLayout)
{
    m_attachedDescriptorSetLayouts.push_back(setLayout);
}
//void createDescriptorSetLayout(VkDescriptorType descType, VkShaderStageFlagBits shaderStage);

void VGraphicsPipeline::attachPushConstant(VkFlags shaderStageBits, uint32_t size, uint32_t offset)
{
    m_attachedPushConstants.push_back(VkPushConstantRange{});
    m_attachedPushConstants.back().offset       = offset;
    m_attachedPushConstants.back().size         = size;
    m_attachedPushConstants.back().stageFlags   = shaderStageBits;
}

void VGraphicsPipeline::setDepthTest(bool enableWrite, bool enableTest, VkCompareOp compareOp)
{
    m_depthStencil.depthTestEnable = enableTest ? VK_TRUE : VK_FALSE;// static_cast<VkBool32>(enableRead);
    m_depthStencil.depthWriteEnable = enableWrite ? VK_TRUE : VK_FALSE; // static_cast<VkBool32>(enableWrite);
    m_depthStencil.depthCompareOp = compareOp;
}

void VGraphicsPipeline::setStencilTest(bool enableTest, VkStencilOpState both)
{
    this->setStencilTest(enableTest, both, both);
}

void VGraphicsPipeline::setStencilTest(bool enableTest, VkStencilOpState front, VkStencilOpState back)
{
    m_depthStencil.stencilTestEnable = enableTest ? VK_TRUE : VK_FALSE;
    m_depthStencil.front = front; // Optional
    m_depthStencil.back = back; // Optional
}

bool VGraphicsPipeline::init(const VRenderPass *renderPass, uint32_t subpass/*, size_t attachmentsCount*/)
{
    VkDevice device = VInstance::device();

    std::vector<VkShaderModule> shaders;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    if(m_specializationInfos.size() < m_specializationData.size())
        m_specializationInfos.resize(m_specializationData.size());

    std::vector<char*> totalSpecData;
    totalSpecData.resize(m_specializationData.size());

    for(size_t i = 0 ; i < m_specializationData.size() ; ++i)
    {
        if(!m_specializationData[i].empty())
        {
            size_t totalSize = 0;
            for(auto entry : m_specializationMapEntries[i])
                totalSize += entry.size;

            totalSpecData[i] = new char[totalSize];

            size_t offset = 0;
            for(size_t j = 0 ; j <  m_specializationMapEntries[i].size() ; ++j)
                for(size_t k = 0 ; k < m_specializationMapEntries[i][j].size ; ++k)
                    totalSpecData[i][offset++] = m_specializationData[i][j][k];

            m_specializationInfos[i].dataSize = totalSize;
            m_specializationInfos[i].pData = totalSpecData[i];
            m_specializationInfos[i].mapEntryCount = static_cast<uint32_t>(m_specializationMapEntries[i].size());
            m_specializationInfos[i].pMapEntries = m_specializationMapEntries[i].data();
        }
    }

    size_t s = 0;
    for(auto shader : m_attachedShaders)
    {
        auto shaderCode = VulkanHelpers::readFile(shader.first);
        shaders.push_back(VulkanHelpers::createShaderModule(shaderCode));

        shaderStages.push_back(VkPipelineShaderStageCreateInfo{});
        shaderStages.back().sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages.back().stage = shader.second;
        shaderStages.back().module = shaders.back();
        shaderStages.back().pName = "main";
        if(s < m_specializationInfos.size())
            shaderStages.back().pSpecializationInfo = &m_specializationInfos[s];

        ++s;
    }

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_staticExtent.width;
    viewport.height = (float) m_staticExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_staticExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = m_cullMode;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    size_t attachmentsCount = renderPass->getColorAttachmentsCount();

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(attachmentsCount);

    m_blendModes.insert(m_blendModes.end(), attachmentsCount - m_blendModes.size(), BlendMode_None);
    m_writeMasks.insert(m_writeMasks.end(), attachmentsCount + 1 - m_writeMasks.size(),
                       VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);

    for(size_t i = 0 ; i < attachmentsCount ; ++i)
    {
        colorBlendAttachments[i] = {};

        if(m_blendModes[i] == BlendMode_Custom)
        {
            colorBlending = m_customBlends[i];
        } else {
            colorBlendAttachments[i].colorWriteMask = m_writeMasks[i];

            switch(m_blendModes[i])
            {
                case BlendMode_Add:
                    colorBlendAttachments[i].blendEnable = VK_TRUE;
                    colorBlendAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                    colorBlendAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
                    colorBlendAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
                    colorBlendAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                    colorBlendAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                    colorBlendAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
                    break;

                case BlendMode_Alpha:
                    colorBlendAttachments[i].blendEnable = VK_TRUE;
                    colorBlendAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                    colorBlendAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    colorBlendAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
                    colorBlendAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                    colorBlendAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                    colorBlendAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
                    break;

                case BlendMode_Accu:
                    colorBlendAttachments[i].blendEnable = VK_TRUE;
                    colorBlendAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
                    colorBlendAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
                    colorBlendAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
                    colorBlendAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
                    colorBlendAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
                    colorBlendAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
                    break;

                default:
                    colorBlendAttachments[i].blendEnable = VK_FALSE;
                    break;
            }
        }
    }

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    ///Could be dynamic, depending on framerate
    colorBlending.blendConstants[3] = 0.5f; //For accumulating, could be a parameter to tweak speed of converging

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_attachedDescriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = m_attachedDescriptorSetLayouts.data();

    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_attachedPushConstants.size());
    pipelineLayoutInfo.pPushConstantRanges = m_attachedPushConstants.data();

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        Logger::error("Failed to create pipeline layout");
        return (false);
    }

    ///Could add this as option (or probably remove it as option).
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount =  1 + !m_onlyScissorIsStatic;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &m_vertexInput;
    pipelineInfo.pInputAssemblyState = &m_inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &m_depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    if(m_onlyScissorIsStatic || (m_staticExtent.height == 0 && m_staticExtent.width == 0))
        pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = renderPass->getVkRenderPass();
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
        return (false);

    for(auto shader : shaders)
        vkDestroyShaderModule(device, shader, nullptr);

    for(auto data : totalSpecData)
        delete[] data;

    return (true);
}

//bool writeDescriptorSet(size_t frameIndex);

void VGraphicsPipeline::bind(VkCommandBuffer &cmb)
{
    vkCmdBindPipeline(cmb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

void VGraphicsPipeline::updateViewport(VkCommandBuffer &cmb, glm::vec2 pos, VkExtent2D extent, bool alsoUpdateScissor)
{
    VkViewport viewport = {};
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.x = pos.x;
    viewport.y = pos.y;
    viewport.width  = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);

    vkCmdSetViewport(cmb, 0, 1, &viewport);

    if(alsoUpdateScissor && !m_onlyScissorIsStatic)
        this->updateScissor(cmb, pos, extent);
}

void VGraphicsPipeline::updateScissor(VkCommandBuffer &cmb, glm::vec2 pos, VkExtent2D extent)
{
    VkRect2D scissor = {};
    scissor.offset = {pos.x, pos.y};
    scissor.extent = extent;

    vkCmdSetScissor(cmb, 0, 1, &scissor);
}

VkPipelineLayout VGraphicsPipeline::getLayout()
{
    return m_pipelineLayout;
}


void VGraphicsPipeline::destroy()
{
    VkDevice device = VInstance::device();

    if(m_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;

    if(m_pipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    m_pipelineLayout = VK_NULL_HANDLE;


    for(auto vect : m_specializationData)
        for(auto p : vect)
            delete[] p;

    m_specializationData.clear();
}

}
