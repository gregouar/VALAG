#ifndef ISOSPRITEENTITY_H
#define ISOSPRITEENTITY_H

#include "Valag/gfx/SceneEntity.h"
#include "Valag/gfx/IsoSpriteModel.h"

namespace vlg
{

struct IsoSpriteEntityUBO {
    glm::mat4 model;
    glm::vec4 color;
};

class IsoSpriteEntity : public SceneEntity
{
    friend class SceneRenderer;

    public:
        IsoSpriteEntity();
        virtual ~IsoSpriteEntity();

        void setColor(Color color);

    protected:
        void updateUBO(SceneRenderer *renderer, size_t frameIndex);
        void cleanup();

        bool checkUpdates(SceneRenderer *renderer, size_t frameIndex);

        IsoSpriteModel *m_model;

    private:
        glm::vec4 m_color;

        std::vector<bool>       m_needToAllocUBO;
        std::vector<bool>       m_needToUpdateUBO;
        std::vector<size_t>     m_UBOIndex;
        std::vector<size_t>     m_UBOVersion;



    /** Static **/
        static bool initRendering();
        static void updateRendering(size_t frameIndex); //Expands UBOs
        static void cleanupRendering();

        static VkDescriptorSetLayout getUBODescriptorSetLayout();

        static DynamicUBODescriptor s_entityUBO;
};

}

#endif // ISOSPRITEENTITY_H
