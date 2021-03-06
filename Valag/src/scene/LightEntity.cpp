#include "Valag/scene/LightEntity.h"

#include "Valag/renderers/SceneRenderer.h"

namespace vlg
{


VkVertexInputBindingDescription LightDatum::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(LightDatum);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 5> LightDatum::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

    size_t i = 0;
    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(LightDatum, position);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(LightDatum, color);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(LightDatum, radius);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(LightDatum, shadowMap);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(LightDatum, shadowShift);
    ++i;

    return attributeDescriptions;
}

LightEntity::LightEntity() : SceneEntity(),
    m_type(LightType_Omni),
    m_direction(0.0,0.0,-1.0),
    m_color(1.0,1.0,1.0,1.0),
    m_radius(100.0),
    m_intensity(1.0),
    m_castShadow(false),
    m_shadowMapExtent(512.0, 512.0)
{
    m_isALight = true;

    m_datum.shadowShift = {0,0};
    this->updateDatum();
}

LightEntity::~LightEntity()
{
    VTexturesManager::freeTexture(m_shadowMap);
}

/*void LightEntity::draw(SceneRenderer *renderer)
{
    renderer->addToLightsVbo(this->getLightDatum());
}*/


void LightEntity::generateRenderingData(SceneRenderingInstance *renderingInstance)
{
    renderingInstance->addToLightsVbo(this->getLightDatum());
}

VTexture LightEntity::generateShadowMap(SceneRenderer* renderer, std::list<ShadowCaster*> &shadowCastersList)
{
    if(!(m_shadowMap.attachment.extent.width  == m_shadowMapExtent.x
      && m_shadowMap.attachment.extent.height == m_shadowMapExtent.y))
        this->recreateShadowMap(renderer);

    //This could cause artifacts since we are using last shadowShift
    renderer->addShadowMapToRender(m_shadowMap.renderTarget, m_datum);

    m_datum.shadowShift = {0,0};

    for(auto shadowCaster : shadowCastersList)
    {
        glm::vec2 shadowShift = shadowCaster->castShadow(renderer, this);
        if(glm::abs(shadowShift.x) > glm::abs(m_datum.shadowShift.x))
            m_datum.shadowShift.x = shadowShift.x;
        if(glm::abs(shadowShift.y) > glm::abs(m_datum.shadowShift.y))
            m_datum.shadowShift.y = shadowShift.y;
    }

    return m_shadowMap.texture;
}

LightType LightEntity::getType()
{
    return m_type;
}

glm::vec3 LightEntity::getDirection()
{
    return m_direction;
}

Color LightEntity::getDiffuseColor()
{
    return m_color;
}

float LightEntity::getRadius()
{
    return m_radius;
}

float LightEntity::getIntensity()
{
    return m_intensity;
}

bool LightEntity::isCastingShadows()
{
    return m_castShadow;
}

void LightEntity::setType(LightType type)
{
    if(m_type != type)
    {
        m_type = type;
        this->updateDatum();
    }
}

void LightEntity::setDirection(glm::vec3 direction)
{
    if(m_direction != direction)
    {
        glm::vec3 oldDirection = m_direction;
        m_direction = direction;
        this->updateDatum();

        this->sendNotification(Notification_UpdateShadow, sizeof(oldDirection), (char*)(&oldDirection));
    }
}

void LightEntity::setDiffuseColor(glm::vec3 color)
{
    this->setDiffuseColor(glm::vec4(color,1.0));
}

void LightEntity::setDiffuseColor(Color color)
{
    if(m_color != color)
    {
        m_color = color;
        this->updateDatum();
    }
}

void LightEntity::setRadius(float radius)
{
    if(m_radius != radius)
    {
        m_radius = radius;
        this->updateDatum();
    }
}

void LightEntity::setIntensity(float intensity)
{
    if(m_intensity != intensity)
    {
        m_intensity = intensity;
        this->updateDatum();
    }
}

void LightEntity::setShadowMapExtent(glm::vec2 extent)
{
    m_shadowMapExtent = extent;
}

void LightEntity::enableShadowCasting()
{
    if(!m_castShadow)
    {
        m_castShadow = true;
        this->updateDatum();
    }
}

void LightEntity::disableShadowCasting()
{
    if(m_castShadow)
    {
        m_castShadow = false;
        this->updateDatum();
    }
}


LightDatum LightEntity::getLightDatum()
{
    return m_datum;
}

/// Protected ///
void LightEntity::notify(NotificationSender *sender, NotificationType type,
                         size_t dataSize, char* data)
{
    if(type == Notification_SceneNodeMoved)
        this->updateDatum();
}

void LightEntity::updateDatum()
{
    if(m_type == LightType_Directional)
        m_datum.position = glm::vec4(m_direction,0.0);
    else if(m_parentNode != nullptr)
        m_datum.position = glm::vec4(m_parentNode->getGlobalPosition(),1.0);

    m_datum.color     = m_color;
    m_datum.color.a  *= m_intensity;

    m_datum.radius    = m_radius;

    if(m_castShadow)
        m_datum.shadowMap = {m_shadowMap.texture.getTextureId(),
                             m_shadowMap.texture.getTextureLayer()};
    else
        m_datum.shadowMap = {0,0};
}

void LightEntity::recreateShadowMap(SceneRenderer* renderer)
{
    VTexturesManager::freeTexture(m_shadowMap);
    VTexturesManager::allocRenderableTexture(m_shadowMapExtent.x, m_shadowMapExtent.y, VK_FORMAT_D24_UNORM_S8_UINT,
                                             renderer->getShadowMapsRenderPass(), &m_shadowMap);
    this->updateDatum();
}


}
