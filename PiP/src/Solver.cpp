#include "Solver.h"

using namespace math;

Solver::Solver()
	: m_continuousCollision(false), m_accumulator(0.f), m_timestep(0.02f), m_gravity(9.8f)
{
}


Solver::~Solver()
{
	m_rigidbodies.clear();
}

void Solver::Update(decimal dt)
{
	//Step through mem allocated bodies

	//Fixed timestep with accumulator (50fps)
	m_accumulator += dt;
	if (m_accumulator > 0.2f) m_accumulator = 0.2f;
	while (m_accumulator > m_timestep) {
		(m_continuousCollision) ? ContinuousStep(m_timestep) : Step(m_timestep);
		m_accumulator -= m_timestep;
	}
	//@UP TO THE GRAPHICS APPLICATION:
	//To create a lerp between this frame and the next, interact with the graphic system.
	//ApproxTransform.position = transform.position + m_velocity*m_accumulator ?
	//float alpha = m_accumulator / m_timestep;
}

void Solver::ContinuousStep(decimal dt) 
{
	//We need to have up to date velocities to perform sweeps
	//1: Perform sweeps, storing collision deltas
	//2: Only acknowledge first collision time
	//3: Step everything upto first collision time
	//4: Compute responses for first colliding pair, everything else retains velocity
	//5: Reperform sweeps  (for(i){for (j = i + 1; ..)}
	//6: If new collisions, repeat
	//7: Apply gravity forces (Collisions are impulse based, friction is force based) *optionally do this at step 0
	for (int i = 0; i < m_rigidbodies.size(); i++) {
		//Semi euler integration
		Rigidbody* rb = m_rigidbodies[i];
		rb->m_acceleration = Vector2(0, -m_gravity / rb->m_mass);
		rb->m_velocity += rb->m_acceleration*dt;
	}
	while ( dt > (decimal) 0.f) {
		decimal firstCollision = 1;//Normalized dt
		Manifold firstManifold = Manifold();
		//Upwards collision check
		for (int i = 0; i < m_rigidbodies.size(); i++) {
			for (int j = i + 1; j < m_rigidbodies.size(); j++) {
				Manifold currentManifold = Manifold();
				if (decimal t = m_rigidbodies[i]->ComputeSweep(m_rigidbodies[j], dt, currentManifold)) {
					//They collide during the frame, store
					if ( t <= firstCollision) {
						firstCollision = t;
						firstManifold = currentManifold;
					}
				}
			}
		}
		//Step everything upto collision time
		decimal timeToStep = dt * firstCollision;
		for (int i = 0; i < m_rigidbodies.size(); i++) {
			//Step according to their velocities (Don't apply gravity or frictional accelerations).
			Rigidbody * rb = m_rigidbodies[i];
			rb->m_position += rb->m_velocity*timeToStep;
			rb->m_rotation += rb->m_angularVelocity*timeToStep;
		}
		dt -= timeToStep;
		//If there was collision
		if (firstManifold.rb1) {
			ComputeResponse(firstManifold);
		}
	}
}

void Solver::Step(decimal dt)
{
	//Integration
	for (int i = 0; i < m_rigidbodies.size(); i++) {
		//Semi euler integration
		Rigidbody* rb = m_rigidbodies[i];
		rb->m_acceleration = Vector2(0, -m_gravity / rb->m_mass);
		rb->m_velocity += rb->m_acceleration * dt;
		rb->m_position += rb->m_velocity * dt;
		rb->m_rotation += rb->m_angularVelocity * dt;
		//TODO: Rotations
	}
	//Upwards collision check
	std::vector<Manifold> manifolds;
	for (int i = 0; i < m_rigidbodies.size(); i++) {
		for (int j = i + 1; j < m_rigidbodies.size(); j++) {
			Manifold currentManifold;
			if (m_rigidbodies[i]->ComputeIntersect(m_rigidbodies[j], currentManifold)) {
				//They collide during the frame, store
				manifolds.push_back(currentManifold);//add manifolds
			}
		}
	}
	//Collision response
	for (Manifold manifold : manifolds) ComputeResponse(manifold);

}

void Solver::ComputeResponse(const Manifold& manifold)
{
	/*// First, find the normalized vector n from the center of
	// circle1 to the center of circle2
	Vector n = circle1.center - circle2.center;
	n.normalize();
	// Find the length of the component of each of the movement
	// vectors along n.
	// a1 = v1 . n
	// a2 = v2 . n
	float a1 = v1.dot(n);
	float a2 = v2.dot(n);

	// Using the optimized version,
	// optimizedP =  2(a1 - a2)
	//              -----------
	//                m1 + m2
	float optimizedP = (2.0 * (a1 - a2)) / (circle1.mass + circle2.mass);

	// Calculate v1', the new movement vector of circle1
	// v1' = v1 - optimizedP * m2 * n
	Vector v1' = v1 - optimizedP * circle2.mass * n;

	// Calculate v1', the new movement vector of circle1
	// v2' = v2 + optimizedP * m1 * n
	Vector v2' = v2 + optimizedP * circle1.mass * n;

	circle1.setMovementVector(v1');
	circle2.setMovementVector(v2');*/

	Rigidbody* rb1 = manifold.rb1;
	Rigidbody* rb2 = manifold.rb2;
	//Collision normal
	Vector2 n = manifold.normal;
	n.Normalize();//Expected to come normalized already

	// Find the length of the component of each of the movement vectors along n.
	//decimal a1 = rb1->m_velocity.Dot(n);
	//decimal a2 = rb2->m_velocity.Dot(n);
	// Using the optimized version,
	// optimizedP =  2(a1 - a2)
	//              -----------
	//                m1 + m2
	//decimal optimizedP = ((decimal)2.f * (a1 - a2)) / (rb1->m_mass + rb2->m_mass);
	// Calculate v1', the new movement vector of circle1
	// v1' = v1 - optimizedP * m2 * n
	//rb1->m_velocity -= n * optimizedP * rb2->m_mass;
	// Calculate v1', the new movement vector of circle1
	// v2' = v2 + optimizedP * m1 * n
	//rb2->m_velocity += n * optimizedP * rb1->m_mass;

	//Use contact points for rotation
	//torque = Force X (Contact point - center of mass);
	//We want to apply angular impulse ie an immediate change in angular velocity
	//Use Vector2Str.Cross>

	//Chris Hecker's physics column (Using j impulse)
	//Norma expected to point to A, e = coefficient of restitution (0 = inelastic, 1 = elastic)
	//r1 = perp of A to point of contact, same r2 
	//j = -((1 + e)*v12 * n)/( n*n*(1/m1 + 1/m2) + (r1*n)^2/I1 + (r2*n)^2/I2 )
	//v1' = v1 + j*n/m1 
	//v2' = v2 - j*n/m2
	//wa' = wa + r1*j*n/I1;
	//wb' = wb - r2*j*n/I2;

	n = -n;//Flip n due to convention
	Vector2 rA = (manifold.contactPoint - rb1->m_position).Perp();
	Vector2 rB = (manifold.contactPoint - rb2->m_position).Perp();
	Vector2 vAB = rb2->m_velocity - rb1->m_velocity;
	decimal e = 1;
	decimal impulse = -(1 + e) * vAB.Dot(n) / (1 / rb1->m_mass + 1 / rb2->m_mass +
		Pow(rA.Dot(n), 2) / rb1->m_inertiaTensor + Pow(rB.Dot(n), 2) / rb2->m_inertiaTensor);
	
	rb1->m_velocity -= impulse * n / rb1->m_mass;
	rb2->m_velocity += impulse * n / rb2->m_mass;
	rb1->m_angularVelocity -= rA.Dot(impulse * n) / rb1->m_inertiaTensor;
	rb2->m_angularVelocity += rB.Dot(impulse * n) / rb2->m_inertiaTensor;

}

Rigidbody * Solver::AddBody(Rigidbody * rb)
{
	//Create default sphere and pass reference, using allocator
	m_rigidbodies.push_back(rb);
	return m_rigidbodies.back();
}