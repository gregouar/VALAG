#ifndef ISOSPRITEENTITY_H
#define ISOSPRITEENTITY_H

#include "Valag/scene/SceneEntity.h"
#include "Valag/scene/ShadowCaster.h"
#include "Valag/scene/IsoSpriteModel.h"

#include "Valag/vulkanImpl/vulkanImpl.h"

namespace vlg
{

struct IsoSpriteDatum
{
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


class IsoSpriteEntity : public ShadowCaster
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

        IsoSpriteDatum getIsoSpriteDatum();
        //virtual void draw(SceneRenderer *renderer);
        virtual void generateRenderingData(SceneRenderingInstance *renderingInstance) override;
        virtual void castShadow(SceneRenderer *renderer, LightEntity* light) override;

        virtual void notify(NotificationSender *sender, NotificationType notification) override;

    protected:
        //void updateUBO(SceneRenderer *renderer, size_t frameIndex);
        void cleanup();
        void updateDatum();


        //bool checkUpdates(SceneRenderer *renderer, size_t frameIndex);
    protected:
        IsoSpriteDatum m_datum;
        IsoSpriteModel *m_spriteModel;

    private:
        ///I'll need to rotate normal and depth accordingly (in shader ?)
        float m_rotation;
        glm::vec4 m_color;
        glm::vec3 m_rmt;
};

}

#endif // ISOSPRITEENTITY_H
