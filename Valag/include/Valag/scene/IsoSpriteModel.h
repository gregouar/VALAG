#ifndef ISOSPRITEMODEL_H
#define ISOSPRITEMODEL_H

#include "Valag/core/NotificationListener.h"
#include "Valag/core/NotificationSender.h"
#include "Valag/Types.h"

#include <vector>

namespace vlg
{

class IsoSpriteModel : public NotificationListener, public NotificationSender
{
    friend class IsoSpriteEntity;

    public:
        IsoSpriteModel();
        virtual ~IsoSpriteModel();

        //void setSize(glm::vec2 size);

        void setMaterial(AssetTypeId materialId);
        void setMaterial(MaterialAsset *material);
        void setSize(glm::vec2 size);
        void setTextureRect(glm::vec2 pos, glm::vec2 extent);
        void setTextureCenter(glm::vec2 pos);

        void setColor(Color color);
        void setRmt(Color rmt);

        MaterialAsset *getMaterial();
        glm::vec2 getSize();
        glm::vec2 getTextureExtent();
        glm::vec2 getTexturePosition();
        glm::vec2 getTextureCenter();

        virtual void notify(NotificationSender* sender, NotificationType notification);

    protected:
        //void updateModel(SceneRenderer *renderer, size_t frameIndex);
        void cleanup();

    private:
        MaterialAsset *m_material;
        std::vector<bool>   m_needToCheckLoading;

        glm::vec2 m_size;
        glm::vec2 m_texturePosition;
        glm::vec2 m_textureExtent;
        glm::vec2 m_textureCenter;

        glm::vec4 m_color;
        glm::vec4 m_rmt;

};

}

#endif // ISOSPRITEMODEL_H
