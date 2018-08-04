#ifndef TESTINGSTATE_H
#define TESTINGSTATE_H

#include "Valag/core/GameState.h"
#include "Valag/utils/Singleton.h"

class TestingState : public vlg::GameState, public Singleton<TestingState>
{
     friend class Singleton<TestingState>;

    public:
        void entered();
        void leaving();
        void revealed();
        void obscuring();

        void handleEvents(const EventsManager *eventsManager);
        void update(/**sf::Time**/);
        void draw(/**sf::RenderTarget* **/);


    protected:
        TestingState();
        virtual ~TestingState();

        void init();

    private:
        bool m_firstEntering;
};

#endif // TESTINGSTATE_H
