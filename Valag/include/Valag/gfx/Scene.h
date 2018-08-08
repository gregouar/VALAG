#ifndef SCENE_H
#define SCENE_H

#include "Valag/Types.h"
#include "Valag/gfx/SceneNode.h"

namespace vlg
{

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

        void askToComputeRenderQueue();

        SceneNode *getRootNode();

        /**RectEntity*     createRectEntity(sf::Vector2f = sf::Vector2f(0,0));
        SpriteEntity*   createSpriteEntity(sf::Vector2i);
        SpriteEntity*   createSpriteEntity(sf::IntRect = sf::IntRect(0,0,0,0));

        Light* createLight(LightType = OmniLight, sf::Vector3f = sf::Vector3f(0,0,-1),
                           sf::Color = sf::Color::White);

        Camera* createCamera(sf::Vector2f viewSize); **/

        void destroyCreatedObject(const ObjectTypeID &);
        void destroyAllCreatedObjects();

        /**virtual sf::Vector2f convertMouseToScene(sf::Vector2i);

        virtual void setCurrentCamera(Camera *);
        virtual void setAmbientLight(sf::Color);

        virtual void setShadowCasting(ShadowCastingType);
        virtual void enableGammaCorrection();
        virtual void disableGammaCorrection();**/

    protected:
        ObjectTypeID generateObjectID();
        void addCreatedObject(const ObjectTypeID &, SceneObject*);

        ///virtual int updateLighting(std::multimap<float, Light*> &lightList, int = -1); //-1 is GL_MAX_LIGHTS


        ///Camera *m_currentCamera;
        SceneNode m_rootNode;

        /**sf::Color m_ambientLight;
        ShadowCastingType m_shadowCastingOption;
        bool m_enableSRGB;

        std::list<SceneEntity*> m_renderQueue;
        //std::list<SceneEntity*> m_staticRenderQueue;
        sf::RenderTarget *m_last_target;**/

    private:
        std::map<ObjectTypeID, SceneObject*> m_createdObjects;
        ObjectTypeID m_curNewId;

        bool m_needToUpdateRenderQueue;

        ///static const sf::Vector2u DEFAULT_SHADOWMAP_SIZE;
};

}

#endif // SCENE_H
