#pragma once
#include "BehaviourNode.h"
#include <vector>

class BehaviourNodeWithChildren : public BehaviourNode
{
protected:
	std::vector<BehaviourNode*>childNodes;
public:
	BehaviourNodeWithChildren(const std::string& nodeName) : BehaviourNode(nodeName) {}
	~BehaviourNodeWithChildren()
	{
		for (auto& i : childNodes)
			delete i;
	}

	void AddChild(BehaviourNode* n)
	{
		childNodes.emplace_back(n);
	}

	void Reset() override
	{
		currentState = Initialise;
		for (auto& i : childNodes)
			i->Reset();
	}
};

