#ifndef SPRITE_H
#define SPRITE_H

#include "Valag/vulkanImpl/VulkanImpl.h"

namespace vlg
{

class DefaultRenderer;

class Sprite
{
    friend class DefaultRenderer;

    public:
        Sprite();
        virtual ~Sprite();

        void setSize(glm::vec2 size);
        void setPosition(glm::vec2 position);
        void setTexture(AssetTypeID textureID);
        void setTextureRect(glm::vec2 pos, glm::vec2 extent);

        AssetTypeID getTexture();

    protected:
        void createAllBuffers();
        void createDrawCommandBuffer();
        void createVertexBuffer();

        void updateModelUBO(DefaultRenderer *renderer, size_t currentFrame);

        void cleanup();

        ///Specifying framebuffer may induce better performances
        VkCommandBuffer getDrawCommandBuffer(DefaultRenderer *renderer, size_t currentFrame, /*VkPipeline pipeline,*/ VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);
        bool recordDrawCommandBuffers(DefaultRenderer *renderer, size_t currentFrame, VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);


        glm::vec2 m_size;
        glm::vec2 m_position;

        AssetTypeID m_texture;

        glm::vec2 m_texturePosition;
        glm::vec2 m_textureExtent;

        bool m_needToCreateBuffers;
        std::vector<bool> m_needToUpdateDrawCommandBuffer;

    private:
        std::vector<VkCommandBuffer>    m_drawCommandBuffers;

        ///Could use multiple buffering if needed to change the vertex buffer... probably not useful for sprite
        ///Could use the same vertexBuffer for all Sprites though, should try to use static here
        ///But then this would mean passing TexCoord in UBO

        VBuffer     m_vertexBuffer;

        size_t      m_modelUBOIndex;

        std::vector<bool>   m_needToAllocModel;
        std::vector<bool>   m_needToUpdateModel;
        uint32_t            m_modelIndex; ///should I put a vector to be sure ?
        std::vector<size_t> m_modelBufferVersion;
};

}

#endif // SPRITE_H