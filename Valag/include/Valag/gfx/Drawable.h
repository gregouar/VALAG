#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "Valag/vulkanImpl/VulkanImpl.h"

namespace vlg
{

class DefaultRenderer;

class Drawable
{
    public:
        Drawable();
        virtual ~Drawable();

        ///Specifying framebuffer may induce better performances
        virtual VkCommandBuffer getDrawCommandBuffer(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass,
                                                     uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);

    protected:
        virtual bool recordDrawCommandBuffers(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer) = 0;

        virtual void createDrawCommandBuffers();


        std::vector<VkCommandBuffer>    m_drawCommandBuffers;

        bool m_needToCreateDrawCMB;
        std::vector<bool> m_needToUpdateDrawCMB;

    private:
};

}

#endif // DRAWABLE_H
