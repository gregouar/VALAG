#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

struct ViewUBO {
    glm::mat4 view;
    glm::vec2 screenOffset;
    glm::vec2 screenSizeFactor;
    glm::vec2 depthOffsetAndFactor;
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
        void setScreenOffset(glm::vec3 offset);

        void setLookAt(glm::vec3 position, glm::vec3 lookAt);
        void setView(glm::mat4 view);
        void setZoom(float zoom);

        VkDescriptorSetLayout   getDescriptorSetLayout();
        VkDescriptorSet         getDescriptorSet(size_t frameIndex);

    protected:
        bool createBuffers(size_t framesCount);
        bool createDescriptorSetLayout();
        bool createDescriptorPool(size_t framesCount);
        bool createDescriptorSets(size_t framesCount);

    private:
        ViewUBO   m_viewUbo;
        glm::vec3 m_position;
        glm::vec3 m_lookAt;
        float m_zoom;

        VkDescriptorPool                m_descriptorPool;
        VkDescriptorSetLayout           m_descriptorSetLayout; ///Could be static...
        std::vector<VkDescriptorSet>    m_descriptorSets;

        std::vector<bool>               m_needToUpdateBuffers;
        std::vector<VBuffer>            m_buffers;
};

}

#endif // RENDERVIEW_H
