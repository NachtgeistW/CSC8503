#pragma once

namespace NCL {
	namespace CSC8503 {
		class Constraint	{
		public:
			Constraint() {}
			virtual ~Constraint() {}

			virtual void UpdateConstraint(float dt) = 0;
		};

		class GameObject;
		class PositionConstraint: public Constraint
		{
		protected:
			GameObject* objectA;
			GameObject* objectB;
			float distance;
		public:
			PositionConstraint(GameObject* a, GameObject* b, float d)
			{
				objectA = a;
				objectB = b;
				distance = d;
			}
			~PositionConstraint() = default;

			void UpdateConstraint(float dt) override
			{
				const auto relativePos = objectA->GetTransform().GetPosition() - objectB->GetTransform().GetPosition();
				const float currentDistance = relativePos.Length();
				const float offset = distance - currentDistance;
				if(abs(offset) > 0.0f)
				{
					const auto offsetDir = relativePos.Normalised();

					const auto physA = objectA->GetPhysicsObject();
					const auto physB = objectB->GetPhysicsObject();

					const auto relativeVelocity = physA->GetLinearVelocity() - physB->GetLinearVelocity();

					const float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

					if(constraintMass > 0.0f)
					{
						// how much of their relative force is affecting the constraint
						const float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);
						constexpr float biasFactor = 0.01f;
						const float bias = -(biasFactor / dt) * offset;

						const float lambda = -(velocityDot + bias) / constraintMass;

						const auto aImpulse = offsetDir * lambda;
						const auto bImpulse = -offsetDir * lambda;

						// multiplied by mass here
						physA->ApplyLinearImpulse(aImpulse);
						physB->ApplyLinearImpulse(bImpulse);
					}
				}
			}
		};
	}
}