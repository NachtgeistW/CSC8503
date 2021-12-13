#include "PushdownAutomate.h"

void PushdownAutomate::TestPushdownAutomata(NCL::Window* w)
{
    NCL::CSC8503::PushdownMachine machine(new IntroScreen());
	while (w->UpdateWindow())
	{
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (!machine.Update(dt))
			return;
	}
}
