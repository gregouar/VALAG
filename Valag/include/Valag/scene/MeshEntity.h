#ifndef MESHENTITY_H
#define MESHENTITY_H

#include <Vulkan/vulkan.h>
#include <array>

#include "Valag/scene/SceneEntity.h"

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

class MeshEntity : public SceneEntity
{
    friend class SceneNode;

    public:
        MeshEntity();
        virtual ~MeshEntity();

        void setMesh(AssetTypeId meshId);
        void setMesh(MeshAsset* mesh);

        void setColor(Color color);
        void setRmt(glm::vec3 rmt);

        MeshDatum getMeshDatum();

        virtual void notify(NotificationSender *sender, NotificationType notification);

    protected:
        void cleanup();
        virtual void draw(SceneRenderer *renderer);
        virtual void updateDatum();

    protected:
        MeshAsset *m_mesh;
        MeshDatum  m_datum;

        Color       m_color;
        glm::vec3   m_rmt;
};

}


#endif // MESHENTITY_H
