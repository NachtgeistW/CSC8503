#pragma once
#include <iostream>

#include "../../Common/Window.h"
#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"

class PushdownAutomateManager
{
	class PauseScreen : public NCL::CSC8503::PushdownState
	{
		PushdownResult OnUpdate(float dt, PushdownState** newState) override
		{
			if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::U))
				return Pop;
			return NoChange;
		}
		void OnAwake() override
		{
			std::cout << "Press U to unpause game!\n";
		}
	};

	class GameScreen : public NCL::CSC8503::PushdownState
	{
	protected:
		int coinsMined = 0;
		float pauseReminder = 1;
		PushdownResult OnUpdate(float dt, PushdownState** newState) override
		{
			pauseReminder -= dt;
			if (pauseReminder < 0)
			{
				std::cout << "Coins mined: " << coinsMined << "\n";
				std::cout << "Press P to pause game, or F1 to return to main menu!\n";
				pauseReminder += 1.0f;
			}
			if (NCL::Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::P))
			{
				*newState = new PauseScreen();
				return Push;
			}
			if (NCL::Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::F1))
			{
				std::cout << "Returning to main menu!\n";
				return Pop;
			}
			if (rand() % 7 == 0)
				coinsMined++;
			return NoChange;
		}
		void OnAwake() override
		{
			std::cout << "Preparing to mine coins!\n";
		}
	};

	class IntroScreen : public NCL::CSC8503::PushdownState
	{
		PushdownResult OnUpdate(float dt, PushdownState** newState) override
		{
			if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::SPACE))
			{
				*newState = new GameScreen;
				return Push;
			}
			if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::ESCAPE))
			{
				return Pop;
			}
			return NoChange;
		}
		void OnAwake() override
		{
			std::cout << "Welcome to a really awesome game!\n";
			std::cout << "Press SPACE to begin or ESCAPE to quit!\n";
		}
	};

	void TestPushdownAutomata(NCL::Window* w);
};
