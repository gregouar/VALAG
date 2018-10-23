#ifndef ISOSPRITEMODEL_H
#define ISOSPRITEMODEL_H

#include "Valag/core/NotificationListener.h"
#include "Valag/core/NotificationSender.h"
#include "Valag/vulkanImpl/VTexturesManager.h"
#include "Valag/Types.h"

#include <vector>

namespace vlg
{

class SceneRenderer;

struct Direction
{
    float x;
    float y;
    float z;

    Direction(glm::vec3 v){x=v.x, y=v.y, z=v.z;}

    bool operator<( Direction const& rhs ) const
    {
        if(x < rhs.x)
            return (true);
        if(x > rhs.x)
            return (false);

        if(y < rhs.y)
            return (true);
        if(y > rhs.y)
            return (false);

        if(z < rhs.z)
            return (true);
        if(z > rhs.z)
            return (false);
        return (false);
    }
};

struct SpriteShadowGenerationDatum
{
    glm::vec3 direction;
    glm::vec3 size;
    //glm::vec2 center;

    glm::vec2 texPos;
    glm::vec2 texExtent;

    glm::uvec2 albedo_texId;
    glm::uvec2 height_texId;
    //glm::uvec2 normal_texId;
    //glm::uvec2 rmt_texId;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions();
};

class IsoSpriteModel : public NotificationListener, public NotificationSender
{
    friend class IsoSpriteEntity;

    public:
        IsoSpriteModel();
        virtual ~IsoSpriteModel();

        IsoSpriteModel( const IsoSpriteModel& ) = delete;
        IsoSpriteModel& operator=( const IsoSpriteModel& ) = delete;


        void setMaterial(AssetTypeId materialId);
        void setMaterial(MaterialAsset *material);
        void setSize(glm::vec2 size);
        void setTextureRect(glm::vec2 pos, glm::vec2 extent);
        void setTextureCenter(glm::vec2 pos);

        void setColor(Color color);
        void setRmt(Color rmt);

        void setShadowMapExtent(glm::vec2 extent);

        MaterialAsset *getMaterial();
        glm::vec2 getSize();
        glm::vec2 getTextureExtent();
        glm::vec2 getTexturePosition();
        glm::vec2 getTextureCenter();
        bool      isReady();

        VTexture getDirectionnalShadow(SceneRenderer *renderer, glm::vec3 direction);
        void updateDirectionnalShadow(glm::vec3 oldDirection, glm::vec3 newDirection);
        void deleteDirectionnalShadow(glm::vec3 direction);

        virtual void notify(NotificationSender* , NotificationType,
                            size_t dataSize = 0, char* data = nullptr) override;

    protected:
        //void updateModel(SceneRenderer *renderer, size_t frameIndex);
        void cleanup();

        //Add some kind of cleaning ?
        VTexture generateDirectionnalShadow(SceneRenderer *renderer, glm::vec3 direction);

    private:
        MaterialAsset *m_material;
        std::vector<bool>   m_needToCheckLoading;

        glm::vec2 m_size;
        glm::vec2 m_texturePosition;
        glm::vec2 m_textureExtent;
        glm::vec2 m_textureCenter;

        glm::vec4 m_color;
        glm::vec4 m_rmt;

        bool m_isReady;

        glm::vec2 m_shadowMapExtent;
        std::map<Direction, std::pair<VRenderableTexture, bool> > m_directionnalShadows;

};

}

#endif // ISOSPRITEMODEL_H
