#ifndef VGRAPHICSPIPELINE_H
#define VGRAPHICSPIPELINE_H

#include <Vulkan/Vulkan.h>
#include <string>
#include <vector>

#include "Valag/Types.h"

namespace vlg
{

class VGraphicsPipeline
{
    public:
        VGraphicsPipeline();
        virtual ~VGraphicsPipeline();

        ///Only before init()
        void createShader(const std::string &shaderPath, VkShaderStageFlagBits shaderStageBit);

        void setVertexInput(size_t vertexBindingCount, VkVertexInputBindingDescription* vertexBindings,
                            size_t vertexAttributeCount, VkVertexInputAttributeDescription* vertexAttributes);

        void setInputAssembly(VkPrimitiveTopology topology, bool restart = false);

        void setDefaultExtent(VkExtent2D extent);

        void setCullMode(VkCullModeFlagBits cullMode);

        void setBlendMode(BlendMode blendMode, size_t attachmentNbr = 0);
        void setWriteMask(VkFlags  writeMask, size_t attachmentNbr = 0);
        void setCustomBlendMode(VkPipelineColorBlendStateCreateInfo blendMode, size_t attachmentsCount = 1);

        void attachDescriptorSetLayout(VkDescriptorSetLayout setLayout);
        //void createDescriptorSetLayout(VkDescriptorType descType, VkShaderStageFlagBits shaderStage);

        void attachPushConstant(VkShaderStageFlagBits shaderStageBit, uint32_t size, uint32_t offset = 0);

        void setDepthTest(bool enableWrite, bool enableTest, VkCompareOp compareOp = VK_COMPARE_OP_GREATER);
        void setStencilTest(bool enableTest, VkStencilOpState both);
        void setStencilTest(bool enableTest, VkStencilOpState front, VkStencilOpState back);

        bool init(VkRenderPass renderPass, uint32_t subpass, size_t attachmentsCount = 1/*, size_t framesCount*/);
        void destroy();

        ///Only after init()
        //bool writeDescriptorSet(size_t frameIndex);

        void bind(VkCommandBuffer &cmb);

        VkPipelineLayout getLayout();

    protected:
        //bool createDescriptorPool();
        //bool createDescriptorSets();

    private:
        std::vector< std::pair<std::string,VkShaderStageFlagBits> > m_attachedShaders;
        VkPipelineVertexInputStateCreateInfo    m_vertexInput;
        VkPipelineInputAssemblyStateCreateInfo  m_inputAssembly;
        VkExtent2D          m_defaultExtent;
        VkCullModeFlagBits  m_cullMode;
        std::vector<BlendMode>                              m_blendModes;
        std::vector<VkFlags >                               m_writeMasks;
        std::vector<VkPipelineColorBlendStateCreateInfo>    m_customBlends;
        std::vector<VkDescriptorSetLayout>      m_attachedDescriptorSetLayouts;
        std::vector<VkPushConstantRange>        m_attachedPushConstants;
        VkPipelineDepthStencilStateCreateInfo   m_depthStencil;

        /*std::vector<VkDescriptorSetLayout> m_createdDescriptorSetLayouts;
        VkDescriptorPool m_descriptorPool;
        std::vector<std::vector<VkDescriptorSet> > m_createdDescriptorSets;*/

        VkPipelineLayout    m_pipelineLayout;
        VkPipeline          m_pipeline;
};

}

#endif // VGRAPHICSPIPELINE_H
