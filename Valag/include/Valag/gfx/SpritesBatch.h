#ifndef SPRITESBATCH_H
#define SPRITESBATCH_H

#include <set>
#include <list>

#include "Valag/gfx/Sprite.h"
#include "Valag/core/NotificationListener.h"

namespace vlg
{

/// I should add option to preserves ordering (disabling auto sorting by material)

class SpritesBatch : public NotificationListener, public Drawable
{
    public:
        SpritesBatch();
        virtual ~SpritesBatch();

        void addSprite(Sprite *sprite);
        bool removeSprite(Sprite *sprite);

        VkCommandBuffer getDrawCommandBuffer(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                             uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);

    protected:
        bool recordDrawCommandBuffers(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                      uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);

        virtual void notify(NotificationSender*, NotificationType);

    private:
       // std::set<Sprite*> m_sprites;
      //  std::list<Sprite*> m_sprites;
       std::map<AssetTypeID, std::set<Sprite*> > m_sortedSprites;
};

}

#endif // SPRITESBATCH_H
