#ifndef SPRITE_H
#define SPRITE_H

#include <glm/glm.hpp>
#include <Vulkan/vulkan.h>

#include "Valag/Types.h"
#include "Valag/gfx/VInstance.h"

namespace vlg
{

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
        void createDrawCommandBuffer();
        void createVertexBuffer();
        void cleanup();

        ///Specifying framebuffer may induce better performances
        VkCommandBuffer recordDrawCommandBuffer(VkPipeline pipeline, VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE);


        glm::vec2 m_size;
        glm::vec2 m_position;

        AssetTypeID m_texture;
        glm::vec2 m_texturePosition;
        glm::vec2 m_textureExtent;

        bool m_needToCreateDrawCommandBuffer;
        bool m_needToUpdateDrawCommandBuffer;
        bool m_needToCreateVertexBuffer;



    private:
        VInstance      *m_creatingVInstance;
        VkCommandBuffer m_drawCommandBuffer; //Would maybe need double buffering
        VkBuffer        m_vertexBuffer;
        VkDeviceMemory  m_vertexBufferMemory;
};

}

#endif // SPRITE_H
