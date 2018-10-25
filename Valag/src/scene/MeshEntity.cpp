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

std::array<VkVertexInputAttributeDescription, 11> MeshDatum::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 11> attributeDescriptions = {};

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

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = d+i;
    attributeDescriptions[i].format = VK_FORMAT_R32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshDatum, texThickness);
    ++i;


    return attributeDescriptions;
}


MeshEntity::MeshEntity() :
    m_mesh(nullptr),
    m_color(1.0,1.0,1.0,1.0),
    m_rmt(1.0,1.0,1.0),
    m_scale(1.0,1.0,1.0)
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
        this->stopListeningTo(m_mesh);
        m_mesh = mesh;
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

void MeshEntity::setScale(float scale)
{
    this->setScale({scale, scale, scale});
}

void MeshEntity::setScale(glm::vec3 scale)
{
    if(m_scale != scale)
    {
        m_scale = scale;
        this->updateDatum();
    }
}

MeshDatum MeshEntity::getMeshDatum()
{
    return m_datum;
}

glm::vec3 MeshEntity::getScale()
{
    return m_scale;
}

void MeshEntity::generateRenderingData(SceneRenderingInstance *renderingInstance)
{
    if(m_mesh != nullptr && m_mesh->isLoaded() && m_datum.albedo_color.a > 0)
        renderingInstance->addToMeshesVbo(m_mesh->getMesh(), this->getMeshDatum());
}

glm::vec2 MeshEntity::castShadow(SceneRenderer *renderer, LightEntity* light)
{
    if(m_mesh == nullptr || !m_mesh->isLoaded())
        return glm::vec2{0.0};

    if(light->getType() == LightType_Directional)
        renderer->addToMeshShadowsVbo(m_mesh->getMesh(), m_datum);

    ///I should compute bounding box and then maxShadowShift
    return glm::vec2{0.0};
}

/*void MeshEntity::draw(SceneRenderer *renderer)
{
    if(m_mesh != nullptr && m_mesh->isLoaded())
        renderer->addToMeshesVbo(m_mesh->getMesh(), this->getMeshDatum());
}*/

void MeshEntity::notify(NotificationSender *sender, NotificationType notification,
                        size_t dataSize, char* data)
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

    m_datum.albedo_color  = m_color;
    m_datum.rmt_color     = m_rmt;
    m_datum.texThickness  = 0.0;

    m_datum.albedo_texId = {};
    m_datum.height_texId = {};
    m_datum.normal_texId = {};
    m_datum.rmt_texId = {};

    MaterialAsset* material = m_mesh->getMaterial();
    if(material != nullptr && material->isLoaded())
    {
        m_datum.albedo_texId = {material->getAlbedoMap().getTextureId(),
                                material->getAlbedoMap().getTextureLayer()};
        m_datum.height_texId = {material->getHeightMap().getTextureId(),
                                material->getHeightMap().getTextureLayer()};
        m_datum.normal_texId = {material->getNormalMap().getTextureId(),
                                material->getNormalMap().getTextureLayer()};
        m_datum.rmt_texId    = {material->getRmtMap().getTextureId(),
                                material->getRmtMap().getTextureLayer()};
        m_datum.rmt_color *= material->getRmtFactor();

        if(!(m_datum.normal_texId.x == 0 && m_datum.normal_texId.y == 0))
            m_datum.texThickness = material->getHeightFactor();
    }

    glm::vec3 scale = m_scale * m_mesh->getScale();

    glm::mat4 modelMatrix = glm::mat4(1.0);

    modelMatrix = glm::translate(modelMatrix, m_parentNode->getGlobalPosition());

    //I should use quaternion (and also inherits rotations from parents)
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().x, glm::vec3(1.0,0.0,0.0));
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().y, glm::vec3(0.0,1.0,0.0));
    modelMatrix = glm::rotate(modelMatrix, m_parentNode->getEulerRotation().z, glm::vec3(0.0,0.0,1.0));

    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::scale(modelMatrix, m_parentNode->getScale());

    m_datum.model_0 = modelMatrix[0];
    m_datum.model_1 = modelMatrix[1];
    m_datum.model_2 = modelMatrix[2];
    m_datum.model_3 = modelMatrix[3];

}

}
