#pragma once
#include <iostream>

#include "TutorialGame.h"
#include "../../Common/Window.h"
#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"

class PushdownAutomateManager
{
public:
	//class EndScreen : public NCL::CSC8503::PushdownState
	//{
	//protected:
 //       NCL::Window* window;
	//	TutorialGame* game;
	//public:
	//	EndScreen(NCL::Window* w, TutorialGame* g) : PushdownState()
	//	{
	//		window = w;
	//		game = g;
	//	}
	//	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	//	{
	//		game->OnGameEnd();
	//		if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::F1))
	//		{
	//			std::cout << "Return to main menu!\n";
	//			game->ResetGameStatue();
	//			return Pop;
	//		}
	//		return NoChange;
	//	}
	//	void OnAwake() override
	//	{
	//		std::cout << "Game End! Press F1 to return to main menu!\n";
	//	    game->SetIsInEnd(true);
	//		if (game->GetIsInLevel1())
	//			game->SetIsLevel1End(true);
	//	}
	//};

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
				game->ResetWorld();
				return Pop;
			}
			return NoChange;
		}
		void OnAwake() override
		{
			std::cout << "Press U to unpause game, F1 to return to main menu!\n";
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
		    NCL::Debug::Print("Press P to Pause", Vector2(5, 20));
			if (game->GetIsInLevel1() && !game->GetIsLevel1End())
			{
				NCL::Debug::Print("Level1", Vector2(5, 15));
			}
			else if (game->GetIsInLevel2())
			{
				NCL::Debug::Print("Level2", Vector2(5, 15));
			}
			else
			{
				NCL::Debug::Print("Press F1 to return to main menu!", Vector2(5, 20));
			    if (NCL::Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::F1))
			    {
					game->ResetWorld();
					return Pop;
			    }
			}

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

			//Game End
			//if (game->GetIsLevel1End())
			//{
			//	*newState = new EndScreen(window, game);
			//	return Push;
			//}
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
			game->UpdateGame(dt);
			NCL::Debug::Print("Welcome to a really awesome game!", Vector2(5, 15));
			NCL::Debug::Print("Press NUM1 to begin Level1, NUM2 to begin Level2,", Vector2(5, 20));
			NCL::Debug::Print("or ESCAPE to quit!", Vector2(5, 25));
			if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM1))
			{
				*newState = new GameScreen(window, game);
				game->SetIsInLevel1(true);
				game->SetIsInMenu(false);
				game->InitGameBoardLevel1();
				return Push;
			}
			else if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM2))
			{
				*newState = new GameScreen(window, game);
				game->SetIsInLevel2(true);
				game->SetIsInMenu(false);
				game->InitGameBoardLevel2();
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
			game->ResetGameStatue();
		}
	};

	void TestPushdownAutomata(NCL::Window* w);
};
