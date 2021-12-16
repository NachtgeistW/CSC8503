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
    std::vector<NCL::Maths::Vector3> pathNodes;
public:
	void TestPathfinding(Vector3 startPos, Vector3 endPos)
	{
        NCL::CSC8503::NavigationPath outPath;
        //NCL::CSC8503::NavigationGrid grid("TestGrid1.txt");
        //NCL::Maths::Vector3 startPos(80, 0, 10);
        //NCL::Maths::Vector3 endPos(80, 0, 80);

        NCL::CSC8503::NavigationGrid grid("TestGrid2.txt");

		bool found = grid.FindPath(startPos, endPos, outPath);
        NCL::Maths::Vector3 pos;
		while (outPath.PopWaypoint(pos))
		{
			pathNodes.push_back(pos);
		}
	}

	void DisplayPathfinding()
	{
		for (int i = 1; i < pathNodes.size(); ++i)
		{
            NCL::Maths::Vector3 a = pathNodes[i - 1];
            NCL::Maths::Vector3 b = pathNodes[i];

            NCL::Debug::DrawLine(a, b, NCL::Vector4(0, 1, 0, 1));
		}
	}

    std::vector<Vector3> GetPathNodes() { return pathNodes; }
    void ClearPathNodes() { pathNodes.clear(); }
};

