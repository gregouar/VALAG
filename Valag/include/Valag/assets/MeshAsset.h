#ifndef MESHASSET_H
#define MESHASSET_H

#include <vector>

#include "Valag/assets/MaterialAsset.h"
#include "Valag/vulkanImpl/VulkanImpl.h"

///I'll need to add handling of thread loading

namespace vlg
{

struct MeshVertex
{
    glm::vec3 pos;
    glm::vec2 uv;

    glm::vec4 albedo_color;
    glm::vec3 rmt_color;

    glm::uvec2 albedo_texId;
    glm::uvec2 height_texId;
    glm::uvec2 normal_texId;
    glm::uvec2 rmt_texId;

    //Will probably need more informations about materials tough

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 8> getAttributeDescriptions();
};

class MeshAsset : public Asset, public NotificationListener
{
    friend class SceneRenderer;

    public:
        MeshAsset();
        MeshAsset(const AssetTypeID);
        virtual ~MeshAsset();

        bool loadFromFile(const std::string &filePath);
        virtual void notify(NotificationSender* , NotificationType);

        float getScale();

    protected:
        bool loadFromXML(TiXmlHandle *);
        bool loadModelFromObj(const std::string &filePath);

        //Cannot generate until textures are loaded
        bool generateModel(const std::vector<glm::vec3> &vertexList,
                           const std::vector<glm::vec2> &uvList,
                           const std::vector<glm::vec2> &indexList);

        VBuffer getVertexBuffer();
        VBuffer getIndexBuffer();
        size_t  getIndexCount();

    private:
         MaterialAsset* m_material; //Could be vector

         float m_scale;
         VBuffer m_vertexBuffer;
         VBuffer m_indexBuffer;
         size_t  m_indexCount;
};


}

#endif // MESHASSET_H
