#pragma once
#include <vector>
#include "../CSC8503Common/Debug.h"
#include "../CSC8503Common/NavigationGrid.h"

namespace NCL
{
    namespace Maths
    {
        class Vector3;
    }
}

class PathfindingManager
{
    std::vector<NCL::Maths::Vector3> testNodes;
public:
	void TestPathfinding()
	{
        NCL::CSC8503::NavigationPath outPath;
        //NCL::CSC8503::NavigationGrid grid("TestGrid1.txt");
        //NCL::Maths::Vector3 startPos(180, 0, 10);
        //NCL::Maths::Vector3 endPos(180, 0, 180);

        NCL::CSC8503::NavigationGrid grid("TestGrid2.txt");
        NCL::Maths::Vector3 startPos(180, 5, 10);
        NCL::Maths::Vector3 endPos(180, 0, 180);

		bool found = grid.FindPath(startPos, endPos, outPath);
        NCL::Maths::Vector3 pos;
		while (outPath.PopWaypoint(pos))
		{
			testNodes.push_back(pos);
		}
	}

	void DisplayPathfinding()
	{
		for (int i = 1; i < testNodes.size(); ++i)
		{
            NCL::Maths::Vector3 a = testNodes[i - 1];
            NCL::Maths::Vector3 b = testNodes[i];

            NCL::Debug::DrawLine(a, b, NCL::Vector4(0, 1, 0, 1));
		}
	}


};

