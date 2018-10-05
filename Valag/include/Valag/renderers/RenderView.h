#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

struct ViewUBO {
    glm::mat4 view;
    glm::mat4 viewInv;
    glm::vec2 screenOffset;
    glm::vec2 screenSizeFactor;
    glm::vec2 depthOffsetAndFactor;
};

class RenderView
{
    public:
        RenderView();
        virtual ~RenderView();

        bool create(size_t imagesCount);
        void destroy();

        void update(size_t imageIndex);

        void setDepthFactor(float depthFactor);

        void setExtent(glm::vec2 extent);
        void setScreenOffset(glm::vec3 offset);

        void setLookAt(glm::vec3 position, glm::vec3 lookAt);
        void setView(glm::mat4 view, glm::mat4 viewInv);
        void setZoom(float zoom);

        glm::vec3 getTranslate();

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
