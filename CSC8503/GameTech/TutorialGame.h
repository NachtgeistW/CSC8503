#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../GameTech/StateGameObject.h"
#include "PathfindingManager.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();
            void ResetWorld();

            virtual void UpdateGame(float dt);
            void InitGameBoardLevel1();
            void InitGameBoardLevel2();

            bool GetIsInMenu()const { return isInMenu; }
			void SetIsInMenu(const bool b) { isInMenu = b; }
			bool GetIsInLevel1() const { return isInLevel1; }
			void SetIsInLevel1(const bool b) { isInLevel1 = b; }
			bool GetIsInLevel2() const { return isInLevel2; }
			void SetIsInLevel2(const bool b) { isInLevel2 = b; }
			bool GetIsInPause() const { return isInPause; }
			void SetIsInPause(const bool b) { isInPause = b; }
			bool GetIsInEnd() const { return isInEnd; }
			void SetIsInEnd(const bool b) { isInEnd = b; }
			void ResetGameStatue()
            {
				isInMenu = true; isInLevel1 = false; isInLevel2 = false; isInPause = false; isInEnd = false;
				isLevel1End = false; isLevel2End = false;
            }
			bool GetIsLevel1End() const { return isLevel1End; }
			void SetIsLevel1End(const bool b) { isLevel1End = b; }

			void InitialiseAssets();
            void OnGameEnd();
            void AiBehaviour(float dt, const vector<Vector3>& pathNodes);
        protected:
			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			void InitCameraLevel1();
            void InitCameraLevel2();
            void UpdateKeys();

			void InitWorldLevel1();
            void InitWorldLevel2();

            void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloorAndWall();
            void InitDefaultFloorAndWallLevel2();
            void InitTargetBall(const Vector3& position);
			void InitOtherBall(const Vector3& position);
            void InitTargetControllerCube(const Vector3& position);
            void InitTargetEnding(const Vector3& position);
            void InitTargetEnemyBall(const Vector3& position);
            void InitBonus();
            void InitConstraintCubeAndRotatingSphere(const Vector3& cubePos, const Vector3& rsPos);
            GameObject* AddRotatingSphereToWorld(const Vector3& position, int radius, float dt,
                                                 float inverseMass);
            void InitGameElementsLevel1();
            void InitGameElementsLevel2();
            void BridgeConstraintTest();

			enum WallExpandAxis { ExpandOnX, ExpandOnZ };
			GameObject* AddWallToWorld(const Vector3& position, const Vector3& size) const;
	
			bool SelectObject();
			void MoveSelectedObject();
            void GameLogicLevel1(float dt);
            void GameLogicLevel2(float dt);
            void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position) const;
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			//Level1 end judgement
			float spentTime = 0;
			float scoreLevel1 = 0;
			bool isLevel1End = false;
			CollisionDetection::CollisionInfo endGameInfo;

			//Level2
			float scoreLevel2 = 0;
			bool isLevel2End = false;
			GameObject* playerBall;
			GameObject* enemyBall;
			GameObject* bonus1, * bonus2;
			PathfindingManager* PfManager;
			int currentPfIndex, nextPfIndex;
			Vector3 lastPlayerPos, lastEnemyPos;

			//Pushdown Automata
			bool isInMenu = true, isInLevel1 = false, isInLevel2 = false, isInPause = false, isInEnd = false;
        };
	}
}

