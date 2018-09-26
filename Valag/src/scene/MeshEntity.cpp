#include "Valag/scene/MeshEntity.h"

#include "Valag/assets/MeshAsset.h"
#include "Valag/renderers/SceneRenderer.h"
#include "Valag/scene/SceneNode.h"

namespace vlg
{


VkVertexInputBindingDescription MeshDatum::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 1;
    bindingDescription.stride = sizeof(MeshDatum);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 6> MeshDatum::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions = {};

    uint32_t i = 0;
    uint32_t d = MeshVertex::getAttributeDescriptions().size();
    uint32_t b = 1;
    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, model_0);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, model_1);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, model_2);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, model_3);
    ++i;


    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, albedo_color);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, rmt_color);
    ++i;


    /*attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, albedo_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, height_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, normal_texId);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(InstanciedIsoSpriteDatum, rmt_texId);
    ++i;*/


    return attributeDescriptions;
}


MeshEntity::MeshEntity() :
    m_mesh(nullptr),
    m_color(1.0,1.0,1.0,1.0),
    m_rmt(1.0,1.0,1.0)
{
    //ctor
}

MeshEntity::~MeshEntity()
{
    //dtor
}

void MeshEntity::setMesh(MeshAsset* mesh)
{
    m_mesh = mesh;
}


void MeshEntity::setColor(Color color)
{
    m_color = color;
}

void MeshEntity::setRmt(glm::vec3 rmt)
{
    m_rmt = rmt;
}

///I should improve this and keep meshDatum stored, only update it when necessary
MeshDatum MeshEntity::getMeshDatum()
{
    MeshDatum datum;

    /*datum.albedo_texId = {0,0};
    datum.height_texId = {0,0};
    datum.normal_texId = {0,0};
    datum.rmt_texId    = {0,0};

    MaterialAsset* material = MaterialsHandler::instance()->getAsset(m_spriteModel->getMaterial());
    if(material->isLoaded())
    {
        datum.albedo_texId = {material->getAlbedoMap().m_textureId,
                              material->getAlbedoMap().m_textureLayer};
        datum.height_texId = {material->getHeightMap().m_textureId,
                              material->getHeightMap().m_textureLayer};
        datum.normal_texId = {material->getNormalMap().m_textureId,
                              material->getNormalMap().m_textureLayer};
        datum.rmt_texId    = {material->getRmtMap().m_textureId,
                              material->getRmtMap().m_textureLayer};
    }*/

    float scale = m_mesh->getScale();

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0), m_parentNode->getGlobalPosition());
    //modelMatrix = glm::translate(modelMatrix, glm::vec3(-m_spriteModel->getTextureCenter(), 0.0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::scale(modelMatrix, m_parentNode->getScale());

    //I should use quaternion
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().x, glm::vec3(1.0,0.0,0.0));
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().y, glm::vec3(0.0,1.0,0.0));
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().z, glm::vec3(0.0,0.0,1.0));

    datum.model_0 = modelMatrix[0];
    datum.model_1 = modelMatrix[1];
    datum.model_2 = modelMatrix[2];
    datum.model_3 = modelMatrix[3];

    datum.albedo_color  = m_color;
    datum.rmt_color     = m_rmt;

    return datum;
}


void MeshEntity::draw(SceneRenderer *renderer)
{
    if(m_mesh != nullptr && m_mesh->isLoaded())
        renderer->addToMeshesVbo(m_mesh, this->getMeshDatum());
}


}
