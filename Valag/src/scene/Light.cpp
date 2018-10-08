#include "Valag/scene/Light.h"

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

std::array<VkVertexInputAttributeDescription, 3> LightDatum::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

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

    return attributeDescriptions;
}

Light::Light() : SceneEntity(),
    m_type(LightType_Omni),
    m_direction(0.0,0.0,-1.0),
    m_color(1.0,1.0,1.0,1.0),
    m_radius(100.0),
    m_intensity(1.0),
    m_castShadow(false)
{
    m_isALight = true;
    this->updateDatum();
}

Light::~Light()
{
    //dtor
}

void Light::draw(SceneRenderer *renderer)
{
    renderer->addToLightsVbo(this->getLightDatum());
}

LightType Light::getType()
{
    return m_type;
}

glm::vec3 Light::getDirection()
{
    return m_direction;
}

Color Light::getDiffuseColor()
{
    return m_color;
}

float Light::getRadius()
{
    return m_radius;
}

float Light::getIntensity()
{
    return m_intensity;
}

void Light::setType(LightType type)
{
    if(m_type != type)
    {
        m_type = type;
        this->updateDatum();
    }
}

void Light::setDirection(glm::vec3 direction)
{
    if(m_direction != direction)
    {
        m_direction = direction;
        this->updateDatum();
    }
}

void Light::setDiffuseColor(glm::vec3 color)
{
    this->setDiffuseColor(glm::vec4(color,1.0));
}

void Light::setDiffuseColor(Color color)
{
    if(m_color != color)
    {
        m_color = color;
        this->updateDatum();
    }
}

void Light::setRadius(float radius)
{
    if(m_radius != radius)
    {
        m_radius = radius;
        this->updateDatum();
    }
}

void Light::setIntensity(float intensity)
{
    if(m_intensity != intensity)
    {
        m_intensity = intensity;
        this->updateDatum();
    }
}

void Light::enableShadowCasting()
{
    m_castShadow = true;
}

void Light::disableShadowCasting()
{
    m_castShadow = false;
}


LightDatum Light::getLightDatum()
{
    return m_datum;
}

/// Protected ///

void Light::notify(NotificationSender *sender, NotificationType type)
{
    if(type == Notification_SceneNodeMoved)
        this->updateDatum();
}

void Light::updateDatum()
{
    if(m_type == LightType_Directionnal)
        m_datum.position = glm::vec4(m_direction,0.0);
    else if(m_parentNode != nullptr)
        m_datum.position = glm::vec4(m_parentNode->getGlobalPosition(),1.0);

    m_datum.color     = m_color;
    m_datum.color.a  *= m_intensity;

    m_datum.radius    = m_radius;
}


}
