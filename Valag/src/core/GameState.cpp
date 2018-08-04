#include "Valag/core/GameState.h"

namespace vlg
{

GameState::GameState()
{
    //ctor
}

GameState::~GameState()
{
    //dtor
}

void GameState::setManager(StatesManager *manager)
{
    m_manager = manager;
}

void GameState::pause()
{
    m_running = false;
}

void GameState::resume()
{
    m_running = true;
}


}
