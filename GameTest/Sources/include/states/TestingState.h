#ifndef TESTINGSTATE_H
#define TESTINGSTATE_H

#include "Valag/core/GameState.h"
#include "Valag/utils/Singleton.h"
#include "Valag/gfx/DefaultRenderer.h"
#include "Valag/gfx/Scene.h"
#include "Valag/gfx/Sprite.h"

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
        void draw(vlg::DefaultRenderer *renderer  /**sf::RenderTarget* **/);


    protected:
        TestingState();
        virtual ~TestingState();

        void init();

    private:
        bool m_firstEntering;

        vlg::Scene *m_scene;
        vlg::Time m_totalTime;

        int m_nbrFps;


        std::vector<vlg::Sprite> m_testingSprites;
};

#endif // TESTINGSTATE_H
