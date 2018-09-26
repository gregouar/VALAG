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

Light::Light() : SceneObject(),
    m_type(LightType_Omni),
    m_direction(0.0,0.0,-1.0),
    m_color(1.0,1.0,1.0,1.0),
    m_radius(100.0),
    m_intensity(1.0)
{
    m_isALight = true;
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
    m_type = type;
}

void Light::setDirection(glm::vec3 direction)
{
    m_direction = direction;
}

void Light::setDiffuseColor(glm::vec3 color)
{
    this->setDiffuseColor(glm::vec4(color,1.0));
}

void Light::setDiffuseColor(Color color)
{
    m_color = color;
}

void Light::setRadius(float radius)
{
    m_radius = radius;
}

void Light::setIntensity(float intensity)
{
    m_intensity = intensity;
}

/// Protected ///

///I should probalby keep it in memory and update only if necessary (like other objects)
LightDatum Light::getLightDatum()
{
    LightDatum datum = {};

    if(m_type == LightType_Directionnal)
        datum.position = glm::vec4(m_direction,0.0);
    else
        datum.position = glm::vec4(m_parentNode->getGlobalPosition(),1.0);

    datum.color     = m_color;
    datum.color.a  *= m_intensity;

    datum.radius    = m_radius;

    return datum;
}


}
