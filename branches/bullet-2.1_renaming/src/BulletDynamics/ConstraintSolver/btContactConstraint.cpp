/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#include "btContactConstraint.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/SimdVector3.h"
#include "btJacobianEntry.h"
#include "btContactSolverInfo.h"
#include "LinearMath/GenMinMax.h"
#include "BulletCollision/NarrowPhaseCollision/btManifoldPoint.h"

#define ASSERT2 assert

//some values to find stable tresholds

float useGlobalSettingContacts = false;//true;
SimdScalar contactDamping = 0.2f;
SimdScalar contactTau = .02f;//0.02f;//*0.02f;







//bilateral constraint between two dynamic objects
void resolveSingleBilateral(btRigidBody& body1, const btSimdVector3& pos1,
                      btRigidBody& body2, const btSimdVector3& pos2,
                      SimdScalar distance, const btSimdVector3& normal,SimdScalar& impulse ,float timeStep)
{
	float normalLenSqr = normal.length2();
	ASSERT2(fabs(normalLenSqr) < 1.1f);
	if (normalLenSqr > 1.1f)
	{
		impulse = 0.f;
		return;
	}
	btSimdVector3 rel_pos1 = pos1 - body1.getCenterOfMassPosition(); 
	btSimdVector3 rel_pos2 = pos2 - body2.getCenterOfMassPosition();
	//this jacobian entry could be re-used for all iterations
	
	btSimdVector3 vel1 = body1.getVelocityInLocalPoint(rel_pos1);
	btSimdVector3 vel2 = body2.getVelocityInLocalPoint(rel_pos2);
	btSimdVector3 vel = vel1 - vel2;
	

	  btJacobianEntry jac(body1.getCenterOfMassTransform().getBasis().transpose(),
		body2.getCenterOfMassTransform().getBasis().transpose(),
		rel_pos1,rel_pos2,normal,body1.getInvInertiaDiagLocal(),body1.getInvMass(),
		body2.getInvInertiaDiagLocal(),body2.getInvMass());

	SimdScalar jacDiagAB = jac.getDiagonal();
	SimdScalar jacDiagABInv = 1.f / jacDiagAB;
	
	  SimdScalar rel_vel = jac.getRelativeVelocity(
		body1.getLinearVelocity(),
		body1.getCenterOfMassTransform().getBasis().transpose() * body1.getAngularVelocity(),
		body2.getLinearVelocity(),
		body2.getCenterOfMassTransform().getBasis().transpose() * body2.getAngularVelocity()); 
	float a;
	a=jacDiagABInv;


	rel_vel = normal.dot(vel);
		

#ifdef ONLY_USE_LINEAR_MASS
	SimdScalar massTerm = 1.f / (body1.getInvMass() + body2.getInvMass());
	impulse = - contactDamping * rel_vel * massTerm;
#else	
	SimdScalar velocityImpulse = -contactDamping * rel_vel * jacDiagABInv;
	impulse = velocityImpulse;
#endif
}




//velocity + friction
//response  between two dynamic objects with friction
float resolveSingleCollision(
	btRigidBody& body1,
	btRigidBody& body2,
	btManifoldPoint& contactPoint,
	const btContactSolverInfo& solverInfo

		)
{

	const btSimdVector3& pos1 = contactPoint.GetPositionWorldOnA();
	const btSimdVector3& pos2 = contactPoint.GetPositionWorldOnB();
    
	
//	printf("distance=%f\n",distance);

	const btSimdVector3& normal = contactPoint.m_normalWorldOnB;

	btSimdVector3 rel_pos1 = pos1 - body1.getCenterOfMassPosition(); 
	btSimdVector3 rel_pos2 = pos2 - body2.getCenterOfMassPosition();
	
	btSimdVector3 vel1 = body1.getVelocityInLocalPoint(rel_pos1);
	btSimdVector3 vel2 = body2.getVelocityInLocalPoint(rel_pos2);
	btSimdVector3 vel = vel1 - vel2;
	SimdScalar rel_vel;
	rel_vel = normal.dot(vel);
	

	SimdScalar Kfps = 1.f / solverInfo.m_timeStep ;

	float damping = solverInfo.m_damping ;
	float Kerp = solverInfo.m_erp;
	
	if (useGlobalSettingContacts)
	{
		damping = contactDamping;
		Kerp = contactTau;
	} 

	float Kcor = Kerp *Kfps;

	//printf("dist=%f\n",distance);

		btConstraintPersistentData* cpd = (btConstraintPersistentData*) contactPoint.m_userPersistentData;
	assert(cpd);

	SimdScalar distance = cpd->m_penetration;//contactPoint.GetDistance();
	

	//distance = 0.f;
	SimdScalar positionalError = Kcor *-distance;
	//jacDiagABInv;
	SimdScalar velocityError = cpd->m_restitution - rel_vel;// * damping;

	
	SimdScalar penetrationImpulse = positionalError * cpd->m_jacDiagABInv;

	SimdScalar	velocityImpulse = velocityError * cpd->m_jacDiagABInv;

	SimdScalar normalImpulse = penetrationImpulse+velocityImpulse;
	
	// See Erin Catto's GDC 2006 paper: Clamp the accumulated impulse
	float oldNormalImpulse = cpd->m_appliedImpulse;
	float sum = oldNormalImpulse + normalImpulse;
	cpd->m_appliedImpulse = 0.f > sum ? 0.f: sum;

	normalImpulse = cpd->m_appliedImpulse - oldNormalImpulse;

	body1.applyImpulse(normal*(normalImpulse), rel_pos1);
	body2.applyImpulse(-normal*(normalImpulse), rel_pos2);
	
	return normalImpulse;
}


float resolveSingleFriction(
	btRigidBody& body1,
	btRigidBody& body2,
	btManifoldPoint& contactPoint,
	const btContactSolverInfo& solverInfo

		)
{

	const btSimdVector3& pos1 = contactPoint.GetPositionWorldOnA();
	const btSimdVector3& pos2 = contactPoint.GetPositionWorldOnB();

	btSimdVector3 rel_pos1 = pos1 - body1.getCenterOfMassPosition(); 
	btSimdVector3 rel_pos2 = pos2 - body2.getCenterOfMassPosition();
	
	btConstraintPersistentData* cpd = (btConstraintPersistentData*) contactPoint.m_userPersistentData;
	assert(cpd);

	float combinedFriction = cpd->m_friction;
	
	SimdScalar limit = cpd->m_appliedImpulse * combinedFriction;
	//if (contactPoint.m_appliedImpulse>0.f)
	//friction
	{
		//apply friction in the 2 tangential directions
		
		{
			// 1st tangent
			btSimdVector3 vel1 = body1.getVelocityInLocalPoint(rel_pos1);
			btSimdVector3 vel2 = body2.getVelocityInLocalPoint(rel_pos2);
			btSimdVector3 vel = vel1 - vel2;
			
			SimdScalar vrel = cpd->m_frictionWorldTangential0.dot(vel);

			// calculate j that moves us to zero relative velocity
			SimdScalar j = -vrel * cpd->m_jacDiagABInvTangent0;
			float total = cpd->m_accumulatedTangentImpulse0 + j;
			GEN_set_min(total, limit);
			GEN_set_max(total, -limit);
			j = total - cpd->m_accumulatedTangentImpulse0;
			cpd->m_accumulatedTangentImpulse0 = total;
			body1.applyImpulse(j * cpd->m_frictionWorldTangential0, rel_pos1);
			body2.applyImpulse(j * -cpd->m_frictionWorldTangential0, rel_pos2);
		}

				
		{
			// 2nd tangent
			btSimdVector3 vel1 = body1.getVelocityInLocalPoint(rel_pos1);
			btSimdVector3 vel2 = body2.getVelocityInLocalPoint(rel_pos2);
			btSimdVector3 vel = vel1 - vel2;

			SimdScalar vrel = cpd->m_frictionWorldTangential1.dot(vel);
			
			// calculate j that moves us to zero relative velocity
			SimdScalar j = -vrel * cpd->m_jacDiagABInvTangent1;
			float total = cpd->m_accumulatedTangentImpulse1 + j;
			GEN_set_min(total, limit);
			GEN_set_max(total, -limit);
			j = total - cpd->m_accumulatedTangentImpulse1;
			cpd->m_accumulatedTangentImpulse1 = total;
			body1.applyImpulse(j * cpd->m_frictionWorldTangential1, rel_pos1);
			body2.applyImpulse(j * -cpd->m_frictionWorldTangential1, rel_pos2);
		}
	} 
	return cpd->m_appliedImpulse;
}
