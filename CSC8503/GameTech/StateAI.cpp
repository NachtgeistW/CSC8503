#include "StateAI.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"
#include "StateGameObject.h"


using namespace NCL;
using namespace CSC8503;

StateAI::StateAI(StateGameObject* stateGameObject, GameObject* bonus,bool enemySeeCoin)
{
    this->stateGameObject = stateGameObject;
    this->bonus = bonus;
    this->enemySeeCoin = enemySeeCoin;

    stateMachine = new StateMachine();
    
    Vector3 rayStartPosition = stateGameObject->GetTransform().GetPosition();

    auto stateA = new NCL::CSC8503::State([&](float dt)->void
        {
            this->StayPut(dt);
        });

    auto stateB = new NCL::CSC8503::State([&](float dt)->void
        {
            this->EarnBonus(dt);
        });

    stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool
        {
            return enemySeeCoin = false;
        }));

    stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool
        {
            return enemySeeCoin = true;
        }));
}

StateAI::~StateAI()
{
    delete stateMachine;
}

void StateAI::Update(float dt)
{
    stateMachine->Update(dt);
}

//Go for bonus

void StateAI::StayPut(float dt)
{
    //Do Nothing;Stay at origin position
}

void StateAI::EarnBonus(float dt)
{
    //Run for Bonus
    Vector3 forceDirection = bonus->GetTransform().GetPosition()
                            - stateGameObject->GetTransform().GetPosition();

    stateGameObject->GetPhysicsObject()->SetLinearVelocity(Vector3(0,10000000,0));

    if (bonus->GetTransform().GetPosition().x == stateGameObject->GetTransform().GetPosition().x 
        && bonus->GetTransform().GetPosition().z == stateGameObject->GetTransform().GetPosition().z
        )
    {
        enemySeeCoin = false;
    }
}



