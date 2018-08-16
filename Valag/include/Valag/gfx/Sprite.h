#ifndef SPRITE_H
#define SPRITE_H

#include <glm/glm.hpp>
#include <Vulkan/vulkan.h>

#include "Valag/Types.h"
#include "Valag/gfx/VInstance.h"

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
        void setTexture();
        void setTextureRect(glm::vec2 pos, glm::vec2 extent);

    protected:
        void createAllBuffers();
        void createDrawCommandBuffer();
        void createVertexBuffer();

        void updateModelUBO(DefaultRenderer *renderer, size_t currentFrame);

        void cleanup();

        ///Specifying framebuffer may induce better performances
        VkCommandBuffer getDrawCommandBuffer(DefaultRenderer *renderer, size_t currentFrame, /*VkPipeline pipeline,*/ VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);
        void recordDrawCommandBuffers(DefaultRenderer *renderer, /*VkPipeline pipeline,*/ VkRenderPass renderPass,
                                                uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);


        glm::vec2 m_size;
        glm::vec2 m_position;

        AssetTypeID m_texture;
        glm::vec2 m_texturePosition;
        glm::vec2 m_textureExtent;

        bool m_needToCreateBuffers;
        bool m_needToUpdateDrawCommandBuffer;

    private:
        VInstance      *m_creatingVInstance;
        std::vector<VkCommandBuffer>    m_drawCommandBuffers;

        ///Could use multiple buffering if needed to change the vertex buffer... probably not useful for sprite
        ///Could use the same vertexBuffer for all Sprites though, should try to use static here
        ///But then this would mean passing TexCoord in UBO
        VkBuffer        m_vertexBuffer;
        VkDeviceMemory  m_vertexBufferMemory;

        size_t      m_modelUBOIndex;

        std::vector<bool> m_needToUpdateModel;
        uint32_t    m_modelIndex; ///should I put a vector to be sure ?
};

}

#endif // SPRITE_H
