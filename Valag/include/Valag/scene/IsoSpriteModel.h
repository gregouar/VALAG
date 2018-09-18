#ifndef ISOSPRITEMODEL_H
#define ISOSPRITEMODEL_H

#include "Valag/Types.h"
//#include "Valag/renderers/SceneRenderer.h"
//#include "Valag/vulkanImpl/DynamicUBODescriptor.h"

#include <vector>

namespace vlg
{

/*struct IsoSpriteModelUBO {
    glm::vec2 texPos;
    glm::vec2 texExt;
};*/

class IsoSpriteModel
{
    friend class IsoSpriteEntity;

    public:
        IsoSpriteModel();
        virtual ~IsoSpriteModel();

        //void setSize(glm::vec2 size);

        void setMaterial(AssetTypeID textureID);
        void setTextureRect(glm::vec2 pos, glm::vec2 extent);
        void setTextureCenter(glm::vec2 pos);

        void setColor(Color color);
        void setRmt(Color rmt);

        AssetTypeID getMaterial();
        glm::vec2 getTextureExtent();
        glm::vec2 getTexturePosition();
        glm::vec2 getTextureCenter();

    protected:
        //void updateModel(SceneRenderer *renderer, size_t frameIndex);
        void cleanup();

    private:
        /// I could (should ?) switch to pointer to textureAsset for more efficiency...
        AssetTypeID m_material;
        std::vector<bool>   m_needToCheckLoading;

        glm::vec2 m_texturePosition;
        glm::vec2 m_textureExtent;
        glm::vec2 m_textureCenter;

        glm::vec4 m_color;
        glm::vec4 m_rmt;



    /** Static **/
      /*  static VkDescriptorSetLayout getUBODescriptorSetLayout();

        static DynamicUBODescriptor s_modelUBO;*/
};

}

#endif // ISOSPRITEMODEL_H
