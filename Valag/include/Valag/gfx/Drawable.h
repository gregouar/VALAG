#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "Valag/vulkanImpl/VulkanImpl.h"

#include "Valag/core/NotificationListener.h"

namespace vlg
{

class DefaultRenderer;

class Drawable : public NotificationListener
{
    public:
        Drawable();
        virtual ~Drawable();

        ///Specifying framebuffer may induce better performances
        virtual VkCommandBuffer getDrawCommandBuffer(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                                     uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);


    protected:
        virtual bool recordDrawCommandBuffers(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer) = 0;

        virtual void createDrawCommandBuffers();
        virtual bool updateDrawCMB(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer);

        void askToUpdateDrawCMB(size_t frameIndex);

        virtual void notify(NotificationSender*, NotificationType);


        std::vector<VkCommandBuffer>    m_drawCommandBuffers;

    private:
        bool m_needToCreateDrawCMB;
        std::vector<bool> m_needToUpdateDrawCMB;
};

}

#endif // DRAWABLE_H
