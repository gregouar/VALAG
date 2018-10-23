#ifndef SPRITESBATCH_H
#define SPRITESBATCH_H

#include <set>
#include <list>

#include "Valag/gfx/Sprite.h"
#include "Valag/core/NotificationListener.h"


#include "Valag/renderers/InstancingRenderer.h"

namespace vlg
{


class SpritesBatch : public Drawable
{
    public:
        SpritesBatch(bool enableSorting = true);
        virtual ~SpritesBatch();

        void addSprite(Sprite *sprite);
        bool removeSprite(Sprite *sprite);

        VkCommandBuffer getDrawCommandBuffer(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                             uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);

        void draw(InstancingRenderer *renderer);

    protected:
        bool recordDrawCommandBuffers(DefaultRenderer *renderer, size_t frameIndex, VkRenderPass renderPass,
                                      uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);

        virtual void notify(NotificationSender* , NotificationType,
                            size_t dataSize = 0, char* data = nullptr) override;

    private:
       // std::set<Sprite*> m_sprites;
      //  std::list<Sprite*> m_sprites;
        bool m_enableSorting;
        std::map<AssetTypeId, std::set<Sprite*> > m_sortedSprites;
};

}

#endif // SPRITESBATCH_H
