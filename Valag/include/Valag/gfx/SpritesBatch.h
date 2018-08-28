#ifndef SPRITESBATCH_H
#define SPRITESBATCH_H

#include <set>

#include "Valag/gfx/Sprite.h"
#include "Valag/core/NotificationListener.h"

namespace vlg
{

class SpritesBatch : public NotificationListener, public Drawable
{
    public:
        SpritesBatch();
        virtual ~SpritesBatch();

        void addSprite(Sprite *sprite);
        bool removeSprite(Sprite *sprite);

        VkCommandBuffer getDrawCommandBuffer(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass,
                                             uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);

    protected:
        bool recordDrawCommandBuffers(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass,
                                      uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);

        virtual void notify(NotificationSender*, NotificationType);

    private:
        std::set<Sprite*> m_sprites;
};

}

#endif // SPRITESBATCH_H
