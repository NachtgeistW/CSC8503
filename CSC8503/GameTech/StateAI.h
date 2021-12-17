#pragma once
#include "../CSC8503Common/GameObject.h"
#include "TutorialGame.h"
namespace NCL
{
    namespace CSC8503
    {
        class StateMachine;
    }
}

class StateAI : public NCL::CSC8503::GameObject
{
public:
    StateAI(StateGameObject* stateGameObject, GameObject* bonus, bool enemySeeCoin);
    ~StateAI();

    virtual void Update(float dt);
protected:
    void EarnBonus(float dt);
    void StayPut(float dt);
    NCL::CSC8503::StateMachine* stateMachine;

    StateGameObject* stateGameObject;
    GameObject* bonus;
    bool enemySeeCoin;

};

