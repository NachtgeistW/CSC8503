#pragma once
#include <iostream>
#include "../CSC8503Common/BehaviourAction.h"
#include "../CSC8503Common/BehaviourSequence.h"

class BehaviourAction;

class BehaviourTreeManager
{
	void TestBehaviourTree()
	{
		float behaviourTimer;
		float distanceToTarget;
		BehaviourAction* findKey = new BehaviourAction("Find Key", [&](float dt, BehaviourState state)->BehaviourState
			{
				if (state == Initialise)
				{
					std::cout << "Looking for a key!\n";
					behaviourTimer = rand() % 100;
					state = Ongoing;
				}
				else if (state == Ongoing)
				{
					behaviourTimer -= dt;
					if (behaviourTimer <= 0.0f)
					{
						std::cout << "Found a key!\n";
						return Success;
					}
				}
				//will be 'ongoing' until success
				return state;
			});
		BehaviourAction* goToRoom = new BehaviourAction("Go To Room", [&](float dt, BehaviourState state)->BehaviourState
			{
				if (state == Initialise)
				{
					std::cout << "Going to the loot room!\n";
					state = Ongoing;
				}
				else if (state == Ongoing)
				{
					distanceToTarget -= dt;
					if (distanceToTarget <= 0.0f)
					{
						std::cout << "Reached room!\n";
						return Success;
					}
				}
				//will be 'ongoing' until success
				return state;
			});
		BehaviourAction* openDoor = new BehaviourAction("Open Door", [&](float dt, BehaviourState state)->BehaviourState
			{
				if (state == Initialise)
				{
					std::cout << "Opening Door!\n";
					return Success;
				}
				return state;
			});
		BehaviourAction* lookForTreasure = new BehaviourAction("Look For Treasure", [&](float dt, BehaviourState state)->BehaviourState
			{
				if (state == Initialise)
				{
					std::cout << "Looking for treasure!\n";
					return Ongoing;
				}
				else if (state == Ongoing)
				{
					bool found = rand() % 2;
					if (found)
					{
						std::cout << "I found some treasure!\n";
						return Success;
					}
					std::cout << "No treasure in here...\n";
					return Failure;
				}
				//will be 'ongoing' until success
				return state;
			});
		BehaviourAction* lookForItems = new BehaviourAction("Look For Items", [&](float dt, BehaviourState state)->BehaviourState
			{
				if (state == Initialise)
				{
					std::cout << "Looking for Items!\n";
					return Ongoing;
				}
				else if (state == Ongoing)
				{
					bool found = rand() % 2;
					if (found)
					{
						std::cout << "I found some items!\n";
						return Success;
					}
					std::cout << "No items in here...\n";
					return Failure;
				}
				//will be 'ongoing' until success
				return state;
			});

		auto sequence = new BehaviourSequence("Room Sequence");
		sequence->AddChild(findKey);
		sequence->AddChild(goToRoom);
		sequence->AddChild(openDoor);

		auto selection = new BehaviourSequence("Loot Selection");
		selection->AddChild(lookForTreasure);
		selection->AddChild(lookForItems);

		auto rootSequence = new BehaviourSequence("Root Sequence");
		rootSequence->AddChild(sequence);
		rootSequence->AddChild(selection);

		for (int i = 0; i < 5; ++i)
		{
			rootSequence->Reset();
			behaviourTimer = 0.0f;
			distanceToTarget = rand() % 250;
			BehaviourState state = Ongoing;
			std::cout << "We're going on an adventure!\n";
			while (state == Ongoing)
			{
				state = rootSequence->Execute(1.0f);//fake dt
			}
			if (state == Success)
				std::cout << "What a successful adventure!\n";
			else if (state == Failure)
				std::cout << "What a waste of time!\n";
		}
        std::cout << "All done!\n";
	}


};

