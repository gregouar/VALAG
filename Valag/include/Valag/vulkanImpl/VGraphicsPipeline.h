#ifndef VGRAPHICSPIPELINE_H
#define VGRAPHICSPIPELINE_H

#include <Vulkan/Vulkan.h>
#include <string>
#include <vector>

#include "Valag/Types.h"
#include "Valag/vulkanImpl/VRenderPass.h"

namespace vlg
{

class VGraphicsPipeline
{
    public:
        VGraphicsPipeline();
        virtual ~VGraphicsPipeline();

        ///Only before init()
        void createShader(const std::string &shaderPath, VkShaderStageFlagBits shaderStageBit);

        void setSpecializationInfo(VkSpecializationInfo &specializationInfo, size_t shaderNbr);
        void addSpecializationDatum(bool value,     size_t shaderNbr); //Need to add more
        void addSpecializationDatum(float value,    size_t shaderNbr);
        void addSpecializationDatum(int value,      size_t shaderNbr);
        void addSpecializationDatum(size_t size, char* data, size_t shaderNbr);

        void setVertexInput(size_t vertexBindingCount, VkVertexInputBindingDescription* vertexBindings,
                            size_t vertexAttributeCount, VkVertexInputAttributeDescription* vertexAttributes);

        void setInputAssembly(VkPrimitiveTopology topology, bool restart = false);

        ///Putting {0,0} (default value) will allow to dynamically change the viewport and scissor
        void setStaticExtent(VkExtent2D extent, bool onlyScissor = false);

        void setCullMode(VkCullModeFlagBits cullMode);

        void setBlendMode(BlendMode blendMode, size_t attachmentNbr = 0);
        void setWriteMask(VkFlags  writeMask, size_t attachmentNbr = 0);
        void setCustomBlendMode(VkPipelineColorBlendStateCreateInfo blendMode, size_t attachmentsCount = 1);

        void attachDescriptorSetLayout(VkDescriptorSetLayout setLayout);
        //void createDescriptorSetLayout(VkDescriptorType descType, VkShaderStageFlagBits shaderStage);

        void attachPushConstant(VkFlags shaderStageBit, uint32_t size, uint32_t offset = 0);

        void setDepthTest(bool enableWrite, bool enableTest, VkCompareOp compareOp = VK_COMPARE_OP_GREATER);
        void setStencilTest(bool enableTest, VkStencilOpState both);
        void setStencilTest(bool enableTest, VkStencilOpState front, VkStencilOpState back);

        bool init(const VRenderPass *renderPass, uint32_t subpass = 0);
        void destroy();

        ///Only after init()
        void bind(VkCommandBuffer &cmb);
        void updateViewport(VkCommandBuffer &cmb, glm::vec2 pos, VkExtent2D extent, bool alsoUpdateScissor = true);
        void updateScissor(VkCommandBuffer &cmb, glm::vec2 pos, VkExtent2D extent);

        VkPipelineLayout getLayout();

    protected:
        std::vector< std::pair<std::string,VkShaderStageFlagBits> > m_attachedShaders;
        std::vector<VkSpecializationInfo>       m_specializationInfos;
        VkPipelineVertexInputStateCreateInfo    m_vertexInput;
        VkPipelineInputAssemblyStateCreateInfo  m_inputAssembly;
        bool                m_onlyScissorIsStatic;
        VkExtent2D          m_staticExtent;
        VkCullModeFlagBits  m_cullMode;
        std::vector<BlendMode>                              m_blendModes;
        std::vector<VkFlags >                               m_writeMasks;
        std::vector<VkPipelineColorBlendStateCreateInfo>    m_customBlends;
        std::vector<VkDescriptorSetLayout>      m_attachedDescriptorSetLayouts;
        std::vector<VkPushConstantRange>        m_attachedPushConstants;
        VkPipelineDepthStencilStateCreateInfo   m_depthStencil;

        std::vector<std::vector<char*> >                     m_specializationData;
        std::vector<std::vector<VkSpecializationMapEntry> >  m_specializationMapEntries;

        VkPipelineLayout    m_pipelineLayout;
        VkPipeline          m_pipeline;
};

}

#endif // VGRAPHICSPIPELINE_H
