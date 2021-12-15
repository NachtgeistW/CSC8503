#pragma once
#include <iostream>

#include "TutorialGame.h"
#include "../../Common/Window.h"
#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"

class PushdownAutomateManager
{
public:
	class PauseScreen : public NCL::CSC8503::PushdownState
	{
	protected:
        NCL::Window* window;
		TutorialGame* game;
	public:
		PauseScreen(NCL::Window* w, TutorialGame* g) : PushdownState()
		{
			window = w;
			game = g;
		}
		PushdownResult OnUpdate(float dt, PushdownState** newState) override
		{
			if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::U))
				return Pop;
			if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::F1))
			{
				std::cout << "Return to main menu!\n";
				game->SetIsInMenu(true);
				game->SetIsInPause(false);
				return Pop;
			}
			return NoChange;
		}
		void OnAwake() override
		{
			std::cout << "Press U to unpause game, NUM1 to return to main menu!\n";
		    game->SetIsInPause(true);
		}
	};

	class GameScreen : public NCL::CSC8503::PushdownState
	{
	protected:
		NCL::Window* window;
		TutorialGame* game;
	public:
		GameScreen(NCL::Window* w, TutorialGame* g) : PushdownState()
		{
			window = w;
			game = g;
		}
		PushdownResult OnUpdate(float dt, PushdownState** newState) override
		{
			game->UpdateGame(dt);

			if (game->GetIsInMenu())
			{
				return Pop;
			}
			game->SetIsInMenu(false);

			if (NCL::Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::P))
			{
				*newState = new PauseScreen(window, game);
				return Push;
			}
			return NoChange;
		}
		void OnAwake() override
		{
			std::cout << "Preparing to play game!\n";
		}
	};

	class IntroScreen : public NCL::CSC8503::PushdownState
	{
	protected:
		NCL::Window* window;
		TutorialGame* game;
	public:
		IntroScreen(NCL::Window* w, TutorialGame* g) : PushdownState()
		{
			window = w;
			game = g;
		}
		PushdownResult OnUpdate(float dt, PushdownState** newState) override
		{
			if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM1))
			{
				*newState = new GameScreen(window, game);
				game->SetIsInLevel1(true);
				game->SetIsInMenu(false);
				return Push;
			}
			else if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM2))
			{
				*newState = new GameScreen(window, game);
				game->SetIsInLevel2(true);
				game->SetIsInMenu(false);
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
			std::cout << "Press NUM1 to begin Level1, NUM2 to begin Level2, or ESCAPE to quit!\n";
			game->SetIsInMenu(true);
		}
	};

	void TestPushdownAutomata(NCL::Window* w);
};
