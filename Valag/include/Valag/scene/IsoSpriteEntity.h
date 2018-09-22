#ifndef ISOSPRITEENTITY_H
#define ISOSPRITEENTITY_H

#include "Valag/scene/SceneEntity.h"
#include "Valag/scene/IsoSpriteModel.h"

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

/*struct IsoSpriteEntityUBO {
    glm::mat4 model;
    glm::vec4 color;
};*/

struct InstanciedIsoSpriteDatum
{
    /*glm::vec4 model_0;
    glm::vec4 model_1;
    glm::vec4 model_2;
    glm::vec4 model_3;*/

    glm::vec3 position;
    float rotation;
    glm::vec3 size;
    glm::vec2 center;

    glm::vec4 albedo_color;
    glm::vec3 rmt_color;
    glm::vec2 texPos;
    glm::vec2 texExtent;
    glm::uvec2 albedo_texId;
    glm::uvec2 height_texId;
    glm::uvec2 normal_texId;
    glm::uvec2 rmt_texId;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 12> getAttributeDescriptions();
};


class IsoSpriteEntity : public SceneEntity
{
    //friend class SceneRenderer;
    friend class SceneNode;

    public:
        IsoSpriteEntity();
        virtual ~IsoSpriteEntity();

        void setRotation(float rotation);
        void setColor(Color color);
        void setRmt(glm::vec3 rmt);
        void setSpriteModel(IsoSpriteModel* model);


        float getRotation();
        Color getColor();
        glm::vec3 getRmt();


        InstanciedIsoSpriteDatum getIsoSpriteDatum();

    protected:
        //void updateUBO(SceneRenderer *renderer, size_t frameIndex);
        void cleanup();

        virtual void draw(SceneRenderer *renderer);

        //bool checkUpdates(SceneRenderer *renderer, size_t frameIndex);

        IsoSpriteModel *m_spriteModel;

    private:
        ///I'll need to rotate normal and depth accordingly (in shader ?)
        float m_rotation;
        glm::vec4 m_color;
        glm::vec3 m_rmt;

        /*std::vector<bool>       m_needToAllocUBO;
        std::vector<bool>       m_needToUpdateUBO;
        std::vector<size_t>     m_UBOIndex;
        std::vector<size_t>     m_UBOVersion;*/



    /** Static **/
        /*static bool initRendering(size_t framesCount);
        static void updateRendering(size_t frameIndex);
        static void cleanupRendering();

        static VkDescriptorSetLayout getUBODescriptorSetLayout();

        static DynamicUBODescriptor s_entityUBO;*/
};

}

#endif // ISOSPRITEENTITY_H
