#ifndef SCENE_H
#define SCENE_H

#include "Valag/Types.h"
#include "Valag/core/NotificationListener.h"
#include "Valag/scene/SceneNode.h"
#include "Valag/scene/CameraObject.h"
#include "Valag/renderers/SceneRenderingInstance.h"
#include "Valag/renderers/SceneRenderingData.h"

namespace vlg
{



/// I need to reimplement methods to create objects (light, sprites, etc)
class Scene : public NotificationListener
{
    public:
        Scene();
        virtual ~Scene();

        virtual void cleanAll();

        virtual void update(const Time &elapsedTime);

        /**virtual void computeRenderQueue();
        virtual sf::View generateView(Camera*);
        virtual void processRenderQueue(sf::RenderTarget*);
        virtual void renderScene(sf::RenderTarget*) = 0;
        virtual void renderShadows(std::multimap<float, Light*> &,const sf::View &,
                                    int = -1); // -1 is GL_MAX_LIGHTS
        virtual void renderEntity(sf::RenderTarget* ,SceneEntity*);**/

        virtual void render(SceneRenderer *renderer, CameraObject *camera);

        SceneNode *getRootNode();

        CameraObject       *createCamera();
        IsoSpriteEntity    *createIsoSpriteEntity(IsoSpriteModel *model = nullptr);
        MeshEntity         *createMeshEntity(MeshAsset *model = nullptr);
        LightEntity        *createLightEntity(LightType type = vlg::LightType_Omni,
                                              Color color = {1.0,1.0,1.0,1.0}, float intensity = 1.0);

        void destroyCreatedObject(const ObjectTypeId);
        void destroyAllCreatedObjects();

        virtual glm::vec2 convertScreenToWorldCoord(glm::vec2 p, CameraObject *cam = nullptr);
        virtual const glm::mat4 &getViewMatrix() const;

        virtual void setAmbientLight(Color color);
        virtual void setEnvironmentMap(TextureAsset *texture);

        /**virtual void setShadowCasting(ShadowCastingType);
        virtual void enableGammaCorrection();
        virtual void disableGammaCorrection();**/

        virtual void generateViewAngle();

        virtual void setViewAngle(float zAngle, float xyAngle);
        //virtual void setCurrentCamera(CameraObject *);

        virtual void notify(NotificationSender* , NotificationType,
                            size_t dataSize = 0, char* data = nullptr) override;

    protected:
        ObjectTypeId generateObjectId();
        void addCreatedObject(const ObjectTypeId, SceneObject*);

        void updateEnvMap();

        ///virtual int updateLighting(std::multimap<float, Light*> &lightList, int = -1); //-1 is GL_MAX_LIGHTS

        SceneNode m_rootNode;

        /**
        ShadowCastingType m_shadowCastingOption;
        bool m_enableSRGB;**/

        float m_zAngle;
        float m_xyAngle;
        glm::mat4   m_viewAngle,    //World to screen transformation matrix (inv is transpose)
                    m_viewAngleInv;// 2D Screen to world transformation matrix

    private:
        std::map<ObjectTypeId, SceneObject*> m_createdObjects;
        ObjectTypeId m_curNewId;

        TextureAsset           *m_envMapAsset;
        SceneRenderingData      m_renderingData;

        ///static const sf::Vector2u DEFAULT_SHADOWMAP_SIZE;
};

}

#endif // SCENE_H
