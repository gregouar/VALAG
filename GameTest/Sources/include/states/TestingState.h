#ifndef TESTINGSTATE_H
#define TESTINGSTATE_H

#include "Valag/core/GameState.h"
#include "Valag/utils/Singleton.h"
#include "Valag/scene/Scene.h"
#include "Valag/gfx/Sprite.h"
#include "Valag/gfx/SpritesBatch.h"
#include "Valag/scene/IsoSpriteEntity.h"
#include "Valag/scene/MeshEntity.h"
#include "Valag/scene/Light.h"

class TestingState : public vlg::GameState, public Singleton<TestingState>
{
     friend class Singleton<TestingState>;

    public:
        void entered();
        void leaving();
        void revealed();
        void obscuring();

        void handleEvents(const EventsManager *eventsManager);
        void update(const vlg::Time &elapsedTime);
        void draw(vlg::RenderWindow *renderWindow);


    protected:
        TestingState();
        virtual ~TestingState();

        void init();

    private:
        bool m_firstEntering;

        vlg::Scene *m_scene;
        vlg::Time m_totalTime;

        int m_nbrFps;

        std::list<vlg::Sprite>  m_testingSprites;
        std::list<vlg::Sprite>  m_testingSpritesInBatch;
        vlg::SpritesBatch       m_testingSpritesBatch;

        vlg::MeshEntity      m_groundSand;

        vlg::IsoSpriteModel  m_treeModel, m_abbeyModel;
        vlg::IsoSpriteEntity m_treeEntity, m_abbeyEntity;
        vlg::MeshAsset      *m_quackMesh;
        std::list<vlg::MeshEntity>  m_quackEntities;
        vlg::SceneNode      *m_treeNode, *m_abbeyNode, *m_quackNode;

        vlg::Light      m_cursorLight,
                        m_sunLight;
        vlg::SceneNode *m_cursorLightNode;
        std::list<vlg::Light> m_secLights;

        vlg::CameraObject   *m_camera;
        vlg::SceneNode      *m_cameraNode;
        glm::vec2            m_camVelocity;
};

#endif // TESTINGSTATE_H
