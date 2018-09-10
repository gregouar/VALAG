#ifndef ISOSPRITEMODEL_H
#define ISOSPRITEMODEL_H

#include "Valag/Types.h"
#include "Valag/gfx/SceneRenderer.h"
#include "Valag/vulkanImpl/DynamicUBODescriptor.h"

namespace vlg
{

struct IsoSpriteModelUBO {
    glm::vec2 texPos;
    glm::vec2 texExt;
};

class IsoSpriteModel
{
    friend class IsoSpriteEntity;

    public:
        IsoSpriteModel();
        virtual ~IsoSpriteModel();

        //void setSize(glm::vec2 size);

        void setTexture(AssetTypeID textureID);
        void setTextureRect(glm::vec2 pos, glm::vec2 extent);
        void setTextureCenter(glm::vec2 pos);

        AssetTypeID getTexture();

    protected:
        void updateModel(SceneRenderer *renderer, size_t frameIndex);
        void cleanup();

    private:
        /// I could (should ?) switch to pointer to textureAsset for more efficiency...
        AssetTypeID m_texture;
        std::vector<bool>   m_needToCheckLoading;

        glm::vec2 m_texturePosition;
        glm::vec2 m_textureExtent;
        glm::vec2 m_textureCenter;


    /** Static **/
        static VkDescriptorSetLayout getUBODescriptorSetLayout();

        static DynamicUBODescriptor s_modelUBO;
};

}

#endif // ISOSPRITEMODEL_H
