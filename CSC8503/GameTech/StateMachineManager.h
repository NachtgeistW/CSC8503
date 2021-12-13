#pragma once
#include <iostream>
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

namespace NCL
{
    namespace CSC8503
    {
        class StateMachine;
    }
}

class StateMachineManager
{
public:
	void TestStateMachine()
	{
        auto testMachine = new NCL::CSC8503::StateMachine();
		int data = 0;
		const auto A = new NCL::CSC8503::State([&](float dt)->void
			{
                std::cout << "I'm in state A!\n";
				data++;
			});
		const auto B = new NCL::CSC8503::State([&](float dt)->void
			{
                std::cout << "I'm in state B!\n";
				data--;
			});
        const auto stateA2B = new NCL::CSC8503::StateTransition(A, B, [&]()->bool
        {
            return data > 10;
        });
        const auto stateB2A = new NCL::CSC8503::StateTransition(A, B, [&]()->bool
        {
            return data < 0;
        });

		testMachine->AddState(A);
		testMachine->AddState(B);
		testMachine->AddTransition(stateA2B);
		testMachine->AddTransition(stateB2A);

		for (int i = 0; i < 100; ++i)
			testMachine->Update(1.0f);
	}

};
