#ifndef MESHASSET_H
#define MESHASSET_H

#include <vector>

#include "Valag/assets/MaterialAsset.h"
#include "Valag/vulkanImpl/VulkanImpl.h"


namespace vlg
{

struct MeshVertex
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;

    /*glm::vec4 albedo_color;
    glm::vec3 rmt_color;

    glm::uvec2 albedo_texId;
    glm::uvec2 height_texId;
    glm::uvec2 normal_texId;
    glm::uvec2 rmt_texId;*/


    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

class MeshAsset : public Asset, public NotificationListener
{
    friend class SceneRenderer;
    friend class MeshesHandler;

    public:
        MeshAsset();
        MeshAsset(const AssetTypeId);
        virtual ~MeshAsset();

        bool loadFromFile(const std::string &filePath);
        virtual void notify(NotificationSender* , NotificationType);

        void setMaterial(AssetTypeId materialId);
        void setMaterial(MaterialAsset* material);

        float getScale();
        MaterialAsset* getMaterial();

    protected:
        bool loadFromXML(TiXmlHandle *);
        bool loadModelFromObj(const std::string &filePath);


        bool generateModel(const std::vector<glm::vec3> &vertexList,
                           const std::vector<glm::vec2> &uvList,
                           const std::vector<glm::vec3> &normalList,
                           const std::vector<glm::vec3> &indexList);

        bool generateModel( std::vector<std::tuple<glm::vec3, glm::vec2, glm::vec3> > &vertexList,
                            std::vector<uint16_t> &indexList);

        VBuffer getVertexBuffer();
        VBuffer getIndexBuffer();
        size_t  getIndexCount();

    private:
         MaterialAsset* m_material; //Could be vector

         float m_scale;
         VBuffer m_vertexBuffer;
         VBuffer m_indexBuffer;
         size_t  m_indexCount;

        bool m_meshLoaded;
        bool m_materialsLoaded;
};


}

#endif // MESHASSET_H
