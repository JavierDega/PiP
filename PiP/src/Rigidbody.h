#pragma once
#include "PiPMath.h"

class Circle;
class Capsule;

class Rigidbody
{
public:
	Rigidbody(math::Vector2 pos = math::Vector2(), decimal rot = (decimal)0.f, math::Vector2 vel = math::Vector2(),
		decimal angVel = (decimal)0.f, math::Vector2 accel = math::Vector2(), decimal mass = 1.f);
	~Rigidbody();

	//Visitor pattern
	virtual bool ComputeIntersect(Rigidbody* rb2, math::Manifold& manifold) = 0;
	virtual bool IntersectWith(Circle* rb2, math::Manifold& manifold) = 0;
	virtual bool IntersectWith(Capsule* rb2, math::Manifold& manifold) = 0;
	virtual decimal ComputeSweep(Rigidbody* rb2, decimal dt, math::Manifold& manifold) = 0;
	virtual decimal SweepWith(Circle* rb2, decimal dt, math::Manifold& manifold) = 0;
	virtual decimal SweepWith(Capsule* rb2, decimal dt, math::Manifold& manifold) = 0;

	math::Vector2 m_position;
	decimal m_rotation;
	math::Vector2 m_velocity;
	decimal m_angularVelocity;
	math::Vector2 m_acceleration;
	decimal m_mass;
	decimal m_inertiaTensor;//Scalar in 2D aka 2nd moment of mass
};