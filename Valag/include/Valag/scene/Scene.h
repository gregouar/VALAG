#ifndef SCENE_H
#define SCENE_H

#include "Valag/Types.h"
#include "Valag/scene/SceneNode.h"
#include "Valag/scene/CameraObject.h"

namespace vlg
{

struct AmbientLightingData
{
    glm::vec4 viewPos;
    glm::vec4 ambientLight;
};


/// I need to reimplement methods to create objects (light, sprites, etc)
class Scene
{
    public:
        Scene();
        virtual ~Scene();

        virtual void cleanAll();

        /**virtual bool initRenderer(sf::Vector2u);**/
        virtual void update(const Time &elapsedTime);
        /**virtual void computeRenderQueue();
        virtual sf::View generateView(Camera*);
        virtual void processRenderQueue(sf::RenderTarget*);
        virtual void renderScene(sf::RenderTarget*) = 0;
        virtual void renderShadows(std::multimap<float, Light*> &,const sf::View &,
                                    int = -1); // -1 is GL_MAX_LIGHTS
        virtual void renderEntity(sf::RenderTarget* ,SceneEntity*);**/

        virtual void render(SceneRenderer *renderer);

        ///This should probably be removed
        void askToComputeRenderQueue();

        SceneNode *getRootNode();

        /**RectEntity*     createRectEntity(sf::Vector2f = sf::Vector2f(0,0));
        SpriteEntity*   createSpriteEntity(sf::Vector2i);
        SpriteEntity*   createSpriteEntity(sf::IntRect = sf::IntRect(0,0,0,0));

        Light* createLight(LightType = OmniLight, sf::Vector3f = sf::Vector3f(0,0,-1),
                           sf::Color = sf::Color::White);**/

        CameraObject* createCamera();

        void destroyCreatedObject(const ObjectTypeId);
        void destroyAllCreatedObjects();

        virtual glm::vec2 convertScreenToWorldCoord(glm::vec2 p, CameraObject *cam = nullptr);

        virtual void setAmbientLight(Color color);

        /**virtual void setShadowCasting(ShadowCastingType);
        virtual void enableGammaCorrection();
        virtual void disableGammaCorrection();**/

        virtual void generateViewAngle();

        virtual void setViewAngle(float zAngle, float xyAngle);
        virtual void setCurrentCamera(CameraObject *);

    protected:
        ObjectTypeId generateObjectId();
        void addCreatedObject(const ObjectTypeId, SceneObject*);

        ///virtual int updateLighting(std::multimap<float, Light*> &lightList, int = -1); //-1 is GL_MAX_LIGHTS


        CameraObject *m_currentCamera;
        SceneNode m_rootNode;

        /**sf::Color m_ambientLight;
        ShadowCastingType m_shadowCastingOption;
        bool m_enableSRGB;

        std::list<SceneEntity*> m_renderQueue;
        //std::list<SceneEntity*> m_staticRenderQueue;
        sf::RenderTarget *m_last_target;**/

        float m_zAngle;
        float m_xyAngle;
        glm::mat4   m_viewAngle,    //World to screen transformation matrix
                    m_viewAngleInv;//, //Screen to world transformation matrix
                   // m_viewProjInv;  //Mouse screen to world matrix

    private:
        std::map<ObjectTypeId, SceneObject*> m_createdObjects;
        ObjectTypeId m_curNewId;

        bool m_needToUpdateRenderQueue;

        AmbientLightingData m_ambientLightingData;

        ///static const sf::Vector2u DEFAULT_SHADOWMAP_SIZE;
};

}

#endif // SCENE_H
