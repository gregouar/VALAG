#ifndef LIGHT_H
#define LIGHT_H

#include "Valag/core/NotificationSender.h"
#include "Valag/scene/SceneEntity.h"
#include "Valag/vulkanImpl/VulkanImpl.h"
#include "Valag/Types.h"

#define LIGHT_TRIANGLECOUNT 6

namespace vlg
{

class SceneRenderer;
class ShadowCaster;

struct LightDatum
{
    glm::vec4 position; //w = 0 for directionnal
    glm::vec4 color; //alpha for intensity
    float     radius;

    glm::uvec2 shadowMap;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};

/// I could try to compute an accurate englobing sphere and use early z-testing to discard pixels

class LightEntity : public SceneEntity, public NotificationSender //, public NotificationListener
{
    friend class SceneNode;

    public:
        LightEntity();
        virtual ~LightEntity();

        LightType   getType();
        glm::vec3   getDirection();
        Color       getDiffuseColor();

        float getRadius();
        float getIntensity();
        bool  isCastingShadows();

        void setType(LightType);
        void setDirection(glm::vec3);
        void setDiffuseColor(glm::vec3);
        void setDiffuseColor(Color);
        void setRadius(float);
        void setIntensity(float);

        void enableShadowCasting();
        void disableShadowCasting();

        void setShadowMapExtent(glm::vec2);

        /*const sf::Texture& GetShadowMap();
        const sf::IntRect& GetShadowMaxShift();

        void RenderShadowMap(const sf::View &);
        void UpdateShadow();*/


        virtual void generateRenderingData(SceneRenderingInstance *renderingInstance);
        virtual VTexture generateShadowMap(SceneRenderer* renderer, std::list<ShadowCaster*> &shadowCastersList);

        LightDatum getLightDatum();

        virtual void notify(NotificationSender* , NotificationType,
                            size_t dataSize = 0, char* data = nullptr) override;


    protected:
        //std::list<ShadowCaster*> *GetShadowCasterList();

        virtual void updateDatum();
        void recreateShadowMap(SceneRenderer* renderer);

    private:
        LightType   m_type;
        glm::vec3   m_direction;
        Color       m_color;

        float m_radius;
        float m_intensity;

        LightDatum  m_datum;

        bool                m_castShadow;
        glm::vec2           m_shadowMapExtent;
        VRenderableTexture  m_shadowMap; //Note that I would actually need one shadow map per renderView if multy viewports... (or I partition shadow map ?)

        /*sf::RenderTexture m_shadowMap;
        sf::IntRect m_shadowMaxShift;
        bool m_requireShadowComputation;

        std::list<ShadowCaster*> m_shadowCasterList;*/
};

}

#endif // LIGHT_H
