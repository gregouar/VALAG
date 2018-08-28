#ifndef SPRITE_H
#define SPRITE_H

#include "Valag/vulkanImpl/VulkanImpl.h"
#include "Valag/gfx/Drawable.h"
#include "Valag/core/NotificationSender.h"

namespace vlg
{
/** For animated sprite, I should work with array of vertexBuffer, having all their texCoord already saved **/

class Sprite : public NotificationSender, public Drawable
{
    friend class DefaultRenderer;
    friend class SpritesBatch;

    public:
        Sprite();
        virtual ~Sprite();

        void setSize(glm::vec2 size);
        void setPosition(glm::vec2 position);
        void setColor(glm::vec4 color);
        void setTexture(AssetTypeID textureID);
        void setTextureRect(glm::vec2 pos, glm::vec2 extent);

        AssetTypeID getTexture();


        ///Specifying framebuffer may induce better performances
        VkCommandBuffer getDrawCommandBuffer(DefaultRenderer *renderer, size_t currentFrame, /*VkPipeline pipeline,*/ VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);

    protected:
        void createAllBuffers();
        //void createDrawCommandBuffer();
        void createVertexBuffer();

        void updateModelUBO(DefaultRenderer *renderer, size_t currentFrame);

        void cleanup();

        bool checkUpdates(DefaultRenderer *renderer, size_t currentFrame);

        bool recordDrawCommandBuffers(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);


        bool recordDrawCMBContent(VkCommandBuffer &commandBuffer, DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer);

    private:
        //std::vector<VkCommandBuffer>    m_drawCommandBuffers;

        ///Could use multiple buffering if needed to change the vertex buffer... probably not useful for sprite
        ///Could use the same vertexBuffer for all Sprites though, should try to use static here
        ///But then this would mean passing TexCoord in UBO

        glm::vec2 m_size;
        glm::vec2 m_position;
        glm::vec4 m_color;

        AssetTypeID m_texture;
        std::vector<bool>   m_needToCheckLoading;

        glm::vec2 m_texturePosition;
        glm::vec2 m_textureExtent;

        bool m_needToCreateBuffers;
       // std::vector<bool> m_needToUpdateDrawCommandBuffer;

        VBuffer     m_vertexBuffer;
        std::vector<bool>   m_needToUpdateVertexBuffer;

        size_t      m_modelUBOIndex;

        std::vector<bool>   m_needToAllocModel;
        std::vector<bool>   m_needToUpdateModel;
        uint32_t            m_modelIndex; ///should I put a vector to be sure ?
        std::vector<size_t> m_modelBufferVersion;
        std::vector<size_t> m_texDescSetVersion;
        bool                m_preventDrawing;
};

}

#endif // SPRITE_H
