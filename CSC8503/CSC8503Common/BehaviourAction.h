#pragma once
#include "BehaviourNode.h"
#include <functional>

typedef std::function <BehaviourState(float, BehaviourState)> BehaviourActionFunc;

class BehaviourAction : public BehaviourNode
{
protected:
	BehaviourActionFunc function;
public:
	BehaviourAction(const std::string& nodeName, BehaviourActionFunc f) : BehaviourNode(nodeName)
	{
		//set the custom function
		function = f;
	}
	BehaviourState Execute(float dt) override
	{
		//call the custom function
		currentState = function(dt, currentState);
		return currentState;
	}
};