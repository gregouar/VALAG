#ifndef MESHENTITY_H
#define MESHENTITY_H

#include <Vulkan/vulkan.h>
#include <array>

#include "Valag/scene/ShadowCaster.h"

namespace vlg
{

struct MeshDatum
{
    glm::vec4 model_0;
    glm::vec4 model_1;
    glm::vec4 model_2;
    glm::vec4 model_3;

    glm::vec4 albedo_color;
    glm::vec3 rmt_color;

    glm::uvec2 albedo_texId;
    glm::uvec2 height_texId;
    glm::uvec2 normal_texId;
    glm::uvec2 rmt_texId;

    float texThickness;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 11> getAttributeDescriptions();
};

class MeshEntity : public ShadowCaster
{
    friend class SceneNode;

    public:
        MeshEntity();
        virtual ~MeshEntity();

        void setMesh(AssetTypeId meshId);
        void setMesh(MeshAsset* mesh);

        void setColor(Color color);
        void setRmt(glm::vec3 rmt);

        void setScale(float scale);
        void setScale(glm::vec3 scale);

        MeshDatum getMeshDatum();
        glm::vec3 getScale();

        virtual void notify(NotificationSender* , NotificationType,
                            size_t dataSize = 0, char* data = nullptr) override;

        virtual void generateRenderingData(SceneRenderingInstance *renderingInstance);

        virtual glm::vec2 castShadow(SceneRenderer *renderer, LightEntity* light) override;


    protected:
        void cleanup();
        virtual void updateDatum();

    protected:
        MeshAsset *m_mesh;
        MeshDatum  m_datum;

        Color       m_color;
        glm::vec3   m_rmt;

        glm::vec3   m_scale;
};

}


#endif // MESHENTITY_H
