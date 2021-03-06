#pragma once
#include "../CSC8503Common/GameWorld.h"
#include <set>

namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem	{
		public:
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();

			void Clear();

			void Update(float dt);

			void UseGravity(bool state) {
				applyGravity = state;
			}

			void SetGlobalDamping(float d) {
				globalDamping = d;
			}

			void SetGravity(const Vector3& g);

			std::set<CollisionDetection::CollisionInfo> GetAllCollisionsInfos () const
            {
			    return allCollisions;
			}
		
		protected:
			void BasicCollisionDetection();
			void BroadPhase();
			void NarrowPhase();

			void ClearForces();

			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt);

			void UpdateConstraints(float dt);
            void ResolveSpringCollision(float dt);

            void UpdateCollisionList();
			void UpdateObjectAABBs();

			void ImpulseResolveCollision(GameObject& a , GameObject&b, CollisionDetection::ContactPoint& p) const;
            void SpecialImpulseResolve(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p)
		    const;

            GameWorld& gameWorld;

			bool	applyGravity;
			Vector3 gravity;
			float	dTOffset;
			float	globalDamping;

			std::set<CollisionDetection::CollisionInfo> allCollisions;

			bool useBroadPhase		= true;
			int numCollisionFrames	= 5;

			std::set<CollisionDetection::CollisionInfo>broadphaseCollisions;
		};
	}
}

