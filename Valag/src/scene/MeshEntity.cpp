#include "Valag/scene/MeshEntity.h"

#include "Valag/assets/MeshAsset.h"

#include "Valag/renderers/SceneRenderer.h"
#include "Valag/scene/SceneNode.h"

#include "Valag/assets/MeshesHandler.h"

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

std::array<VkVertexInputAttributeDescription, 10> MeshDatum::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 10> attributeDescriptions = {};

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


    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, albedo_texId);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, height_texId);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, normal_texId);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, rmt_texId);
    ++i;


    return attributeDescriptions;
}


MeshEntity::MeshEntity() :
    m_mesh(nullptr),
    m_color(1.0,1.0,1.0,1.0),
    m_rmt(1.0,1.0,1.0)
{
    this->updateDatum();
}

MeshEntity::~MeshEntity()
{
    //dtor
}

void MeshEntity::setMesh(AssetTypeId meshId)
{
    this->setMesh(MeshesHandler::instance()->getAsset(meshId));
}

void MeshEntity::setMesh(MeshAsset* mesh)
{
    if(m_mesh != mesh)
    {
        if(m_mesh != nullptr)
            this->stopListeningTo(m_mesh);
        m_mesh = mesh;
        if(m_mesh != nullptr)
            this->startListeningTo(m_mesh);
        this->updateDatum();
    }
}


void MeshEntity::setColor(Color color)
{
    if(m_color != color)
    {
        m_color = color;
        this->updateDatum();
    }
}

void MeshEntity::setRmt(glm::vec3 rmt)
{
    if(m_rmt != rmt)
    {
        m_rmt = rmt;
        this->updateDatum();
    }
}

MeshDatum MeshEntity::getMeshDatum()
{
    return m_datum;
}


void MeshEntity::draw(SceneRenderer *renderer)
{
    if(m_mesh != nullptr && m_mesh->isLoaded())
        renderer->addToMeshesVbo(m_mesh->getMesh(), this->getMeshDatum());
}

void MeshEntity::notify(NotificationSender *sender, NotificationType notification)
{
    if(notification == Notification_AssetLoaded ||
       notification == Notification_TextureChanged ||
       notification == Notification_SceneNodeMoved)
        this->updateDatum();

    if(notification == Notification_SenderDestroyed)
    {
        if(sender == m_mesh)
            m_mesh = nullptr;
    }
}

void MeshEntity::updateDatum()
{
    m_datum = {};
    if(m_mesh == nullptr || m_parentNode == nullptr)
        return;

    MaterialAsset* material = m_mesh->getMaterial();
    if(material != nullptr && material->isLoaded())
    {
        m_datum.albedo_texId = {material->getAlbedoMap().m_textureId,
                              material->getAlbedoMap().m_textureLayer};
        m_datum.height_texId = {material->getHeightMap().m_textureId,
                              material->getHeightMap().m_textureLayer};
        m_datum.normal_texId = {material->getNormalMap().m_textureId,
                              material->getNormalMap().m_textureLayer};
        m_datum.rmt_texId    = {material->getRmtMap().m_textureId,
                              material->getRmtMap().m_textureLayer};
    }

    float scale = m_mesh->getScale();

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0), m_parentNode->getGlobalPosition());
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::scale(modelMatrix, m_parentNode->getScale());

    //I should use quaternion
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().x, glm::vec3(1.0,0.0,0.0));
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().y, glm::vec3(0.0,1.0,0.0));
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().z, glm::vec3(0.0,0.0,1.0));

    m_datum.model_0 = modelMatrix[0];
    m_datum.model_1 = modelMatrix[1];
    m_datum.model_2 = modelMatrix[2];
    m_datum.model_3 = modelMatrix[3];

    m_datum.albedo_color  = m_color;
    m_datum.rmt_color     = m_rmt;
}

}
