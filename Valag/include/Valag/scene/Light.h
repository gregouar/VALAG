#ifndef LIGHT_H
#define LIGHT_H

#include "Valag/scene/SceneEntity.h"
#include "Valag/vulkanImpl/VulkanImpl.h"
#include "Valag/Types.h"

#define LIGHT_TRIANGLECOUNT 6

namespace vlg
{

class SceneRenderer;

struct LightDatum
{
    glm::vec4 position;
    glm::vec4 color; //alpha for intensity
    float     radius;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

/// I could try to compute an accurate englobing sphere and use early z-testing to discard pixels

class Light : public SceneEntity //, public NotificationListener
{
    friend class SceneNode;

    public:
        Light();
        virtual ~Light();

        LightType   getType();
        glm::vec3   getDirection();
        Color       getDiffuseColor();

        float getRadius();
        float getIntensity();
        //bool  isCastShadowEnabled();

        void setType(LightType);
        void setDirection(glm::vec3);
        void setDiffuseColor(glm::vec3);
        void setDiffuseColor(Color);
        void setRadius(float);
        void setIntensity(float);

        void enableShadowCasting();
        void disableShadowCasting();

        /*void setShadowMapSize(unsigned int x, unsigned int y );
        void setShadowMapSize(sf::Vector2u );

        const sf::Texture& GetShadowMap();
        const sf::IntRect& GetShadowMaxShift();

        void RenderShadowMap(const sf::View &);
        void UpdateShadow();*/

        //virtual void notify(NotificationSender*, NotificationType);

        //static int GetMaxNbrLights();


        virtual void generateRenderingData(SceneRenderingInstance *renderingInstance);
        LightDatum getLightDatum();

        virtual void notify(NotificationSender *sender, NotificationType type);


    protected:
        //std::list<ShadowCaster*> *GetShadowCasterList();

        virtual void updateDatum();

    private:
        LightType   m_type;
        glm::vec3   m_direction;
        Color       m_color;

        float m_radius;
        float m_intensity;

        LightDatum  m_datum;

        bool m_castShadow;
        /*sf::RenderTexture m_shadowMap;
        sf::IntRect m_shadowMaxShift;
        bool m_requireShadowComputation;

        std::list<ShadowCaster*> m_shadowCasterList;*/
};

}

#endif // LIGHT_H
