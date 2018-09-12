#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

struct ViewUBO {
    glm::mat4 view;
};

class RenderView
{
    public:
        RenderView();
        virtual ~RenderView();

        bool create(size_t framesCount);
        void destroy();

        void update(size_t frameIndex);

        void setDepthFactor(float depthFactor);
        void setExtent(glm::vec2 extent);
        void setPosition(glm::vec2 position);

        VkDescriptorSetLayout   getDescriptorSetLayout();
        VkDescriptorSet         getDescriptorSet(size_t frameIndex);

    protected:
        bool createBuffers(size_t framesCount);
        bool createDescriptorSetLayout();
        bool createDescriptorPool(size_t framesCount);
        bool createDescriptorSets(size_t framesCount);

    private:
        float     m_depthFactor;
        glm::vec2 m_extent;
        glm::vec2 m_position;

        VkDescriptorPool                m_descriptorPool;
        VkDescriptorSetLayout           m_descriptorSetLayout; ///Could be static...
        std::vector<VkDescriptorSet>    m_descriptorSets;

        std::vector<bool>               m_needToUpdateBuffers;
        std::vector<VBuffer>            m_buffers;
};

}

#endif // RENDERVIEW_H
