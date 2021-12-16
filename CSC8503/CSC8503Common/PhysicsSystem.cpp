#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "../../Common/Quaternion.h"

#include "Constraint.h"

#include "Debug.h"

#include <functional>
using namespace NCL;
using namespace CSC8503;

/*

These two variables help define the relationship between positions
and the forces that are added to objects to change those positions

*/

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= false;
	useBroadPhase	= false;	
	dTOffset		= 0.0f;
	globalDamping	= 0.995f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
}

PhysicsSystem::~PhysicsSystem()	{
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	allCollisions.clear();
}

/*

This is the core of the physics engine update

*/
int constraintIterationCount = 10;

//This is the fixed timestep we'd LIKE to have
const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

/*
This is the fixed update we actually have...
If physics takes too long it starts to kill the framerate, it'll drop the 
iteration count down until the FPS stabilises, even if that ends up
being at a low rate. 
*/
int realHZ		= idealHZ;
float realDT	= idealDT;

void PhysicsSystem::Update(float dt) {	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B)) {
		useBroadPhase = !useBroadPhase;
		std::cout << "Setting broadphase to " << useBroadPhase << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::I)) {
		constraintIterationCount--;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::O)) {
		constraintIterationCount++;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	GameTimer t;
	t.GetTimeDeltaSeconds();

	if (useBroadPhase) {
		UpdateObjectAABBs();
	}

	while(dTOffset >= realDT) {
		IntegrateAccel(realDT); //Update accelerations from external forces
		if (useBroadPhase) {
			BroadPhase();
			NarrowPhase();
		}
		else {
			BasicCollisionDetection();
		}

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = realDT /  (float)constraintIterationCount;
		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);	
		}
		IntegrateVelocity(realDT); //update positions from new velocity changes

		dTOffset -= realDT;
	}

	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions

	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();

	//Uh oh, physics is taking too long...
	if (updateTime > realDT) {
		realHZ /= 2;
		realDT *= 2;
		std::cout << "Dropping iteration count due to long physics time...(now " << realHZ << ")\n";
	}
	else if(dt*2 < realDT) { //we have plenty of room to increase iteration count!
		int temp = realHZ;
		realHZ *= 2;
		realDT /= 2;

		if (realHZ > idealHZ) {
			realHZ = idealHZ;
			realDT = idealDT;
		}
		if (temp != realHZ) {
			std::cout << "Raising iteration count due to short physics time...(now " << realHZ << ")\n";
		}
	}
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a 
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
	for (auto i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}
		(*i).framesLeft = (*i).framesLeft - 1;
		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	//gameWorld.OperateOnContents(
	//	[](GameObject* g) {
	//		g->UpdateBroadphaseAABB();
	//	}
	//);
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i)
		(*i)->UpdateBroadphaseAABB();
}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset 
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {
	vector<GameObject*>::const_iterator first;
	vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for(auto i = first; i!= last; ++i)
	{
		if ((*i)->GetPhysicsObject() == nullptr)
			continue;
		for (auto j = i + 1; j!= last; ++j)
		{
			if ((*j)->GetPhysicsObject() == nullptr)
				continue;
			CollisionDetection::CollisionInfo info;
			if(CollisionDetection::ObjectIntersection(*i, *j, info))
			{
				std::cout << "Collision between " << (*i)->GetName() << " and " << (*j)->GetName() << std::endl;
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
			}
		}
	}
}

/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out. 

*/
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	//TODO:Rotate
	//if (a.GetWorldID() == 101 && b.GetWorldID() == 100 || a.GetWorldID() == 100 && b.GetWorldID() == 101)
	//{
	//	SpecialImpulseResolve(a, b, p);
	//	return;
	//}

	const auto physA = a.GetPhysicsObject();
	const auto physB = b.GetPhysicsObject();
	auto &transformA = a.GetTransform();
	auto &transformB = b.GetTransform();

    const float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	//Is there any calculation necessary for two static objects?
	if(totalMass == 0)
		return;

    //Separate them out using projection
	transformA.SetPosition(transformA.GetPosition() - p.normal * p.penetration * (physA->GetInverseMass() / totalMass));
	transformB.SetPosition(transformB.GetPosition() + p.normal * p.penetration * (physB->GetInverseMass() / totalMass));

	const auto relativeA = p.localA;
	const auto relativeB = p.localB;

	const auto angVelocityA = Vector3::Cross(physA->GetAngularVelocity(), relativeA);
	const auto angVelocityB = Vector3::Cross(physB->GetAngularVelocity(), relativeB);

	const auto fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	const auto fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	const auto contactVelocity = fullVelocityB - fullVelocityA;

	const float impulseForce = Vector3::Dot(contactVelocity, p.normal);

	//整惯性的东西
	auto inertiaA = Vector3::Cross(physA->GetInertiaTensor() * Vector3::Cross(relativeA, p.normal), relativeA);
	auto inertiaB = Vector3::Cross(physB->GetInertiaTensor() * Vector3::Cross(relativeB, p.normal), relativeB);
	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);

	float cRestitution = 0.66f; //disperse some kinetic energy
	float j = -(1.0f + cRestitution) * impulseForce / (totalMass + angularEffect);
	Vector3 fullImpulse = p.normal * j;

	//Apply the liner and angular impulses
	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);

	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, fullImpulse));

	//Elasticity
    constexpr float k = 100.0f;
    const auto force = p.normal * p.penetration * -k;
	if (physA->GetInverseMass() != 0.0f)
		physA->AddForceAtPosition(-force, p.localA);
	if (physB->GetInverseMass() != 0.0f)
		physB->AddForceAtPosition(force, p.localB);

}

void PhysicsSystem::SpecialImpulseResolve(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const
{
    if (a.GetWorldID() == 100)
    {
		a.GetPhysicsObject()->ApplyLinearImpulse(Vector3(5, 0, 0));
		a.GetPhysicsObject()->ApplyAngularImpulse(Vector3(5, 0, 0));
    }
	else if (b.GetWorldID() == 100)
	{
		b.GetPhysicsObject()->ApplyLinearImpulse(Vector3(5, 0, 0));
		b.GetPhysicsObject()->ApplyAngularImpulse(Vector3(5, 0, 0));
	}
}

/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to. 

*/

//宽阶段
void PhysicsSystem::BroadPhase() {
	broadphaseCollisions.clear();
	QuadTree<GameObject*>tree(Vector2(1024, 1024), 7, 6);

	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i)
	{
		Vector3 halfSizes;
		if(!(*i)->GetBroadphaseAABB(halfSizes))
			continue;
		Vector3 pos = (*i)->GetTransform().GetPosition();
		tree.Insert(*i, pos, halfSizes);
	}
	tree.OperateOnContents([&](std::list<QuadTreeEntry<GameObject*>>& data)
		{
			CollisionDetection::CollisionInfo info;
			for (auto i = data.begin(); i != data.end(); ++i)
			{
				for (auto j = std::next(i); j != data.end(); ++j)
				{
					info.a = min((*i).object, (*j).object);
					info.b = max((*i).object, (*j).object);
					broadphaseCollisions.insert(info);
				}
			}
		});
}

/*
The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/

//窄阶段
void PhysicsSystem::NarrowPhase() {
	for (auto info : broadphaseCollisions)
	{
		if(CollisionDetection::ObjectIntersection(info.a, info.b, info))
		{
			info.framesLeft = numCollisionFrames;
			ImpulseResolveCollision(*info.a, *info.b, info.point);
			allCollisions.insert(info);//insert into the main set
		}
	}
}

/*
整合加速度
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc. 

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i)
	{
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr)
			// No physics object for this GameObject !
			continue;
		const float inverseMass = object->GetInverseMass();

		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;	// acceleration

		//Don't move infinitely heavy things
		if (applyGravity && inverseMass > 0)
			accel += gravity;

		//integrate accel
		linearVel += accel * dt;
		object->SetLinearVelocity(linearVel);

		//Angular stuff
		auto torque = object->GetTorque();//力矩
		auto angVel = object->GetAngularVelocity();//角速度

		//update tensor vs orientation
		object->UpdateInertiaTensor();

		auto angAccel = object->GetInertiaTensor() * torque;//角加速度 = 惯性张量 * 力矩

	    angVel += angAccel * dt;
		object->SetAngularVelocity(angVel);
	}
}
/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
    const float frameLinearDamping = 1.0f - 0.4f * dt;

	for (auto i = first; i != last; ++i)
	{
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr)
			continue;
		Transform& transform = (*i)->GetTransform();
		//Position stuff
		Vector3 position = transform.GetPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		transform.SetPosition(position);
		//Linear Damping线性阻尼
		linearVel = linearVel * frameLinearDamping;
		object->SetLinearVelocity(linearVel);

		//Orientation Stuff
		auto orientation = transform.GetOrientation();//方向
		auto angVel = object->GetAngularVelocity();//角速度

		orientation = orientation + Quaternion(angVel * dt * 0.5f, 0.0f) * orientation;

		//Damp the angular velocity
        const float frameAngularDamping = 1.0f - 0.4f * dt;//角阻尼
		angVel = angVel * frameAngularDamping;
		object->SetAngularVelocity(angVel);
	}
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
	gameWorld.OperateOnContents(
		[](GameObject* o) {
			o->GetPhysicsObject()->ClearForces();
		}
	);
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc. 

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}

/*
The current ImpulseResolveCollision uses a combination of the projection and impulse methods to
maintain the consistency of the objects in the scene. This tutorial also discussed the ideal of
using a spring calculation to change the accelerations of the objects, rather than position or
velocity. Try making a ResolveSpringCollision method which uses Hooke’s spring calculations to
move the objects - remember that a spring has a resting length, and a spring coefficient to indicate
how quickly the spring should ’snap’ back to its resting length when stretched or compressed.
*/
//TODO:finish it
void PhysicsSystem::ResolveSpringCollision(float dt)
{
    
}