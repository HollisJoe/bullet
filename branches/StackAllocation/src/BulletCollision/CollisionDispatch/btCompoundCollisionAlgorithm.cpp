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

#include "BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"
#include "BulletCollision/BroadphaseCollision/btDbvt.h"
#include "BulletCollision/BroadphaseCollision/btQuantizedBvh.h"
#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btAabbUtil2.h"
#include "btManifoldResult.h"

btCompoundCollisionAlgorithm::btCompoundCollisionAlgorithm( const btCollisionAlgorithmConstructionInfo& ci)
:btActivatingCollisionAlgorithm(ci, false),
m_isSwapped(ci.m_isSwapped)
{
	const btCollider* colObj = m_isSwapped? ci.m_colObj1 : ci.m_colObj0;
	btAssert (colObj->getCollisionShape()->isCompound());
	
	const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(colObj->getCollisionShape());
	m_compoundShapeRevision = compoundShape->getUpdateRevision();
	
	//preallocateChildAlgorithms(body0,body1);
	m_childCollisionAlgorithms.resize(compoundShape->getNumChildShapes());
}

void	btCompoundCollisionAlgorithm::preallocateChildAlgorithms(btDispatcher* dispatcher, const btCollider& body0,const btCollider& body1)
{
	const btCollider& colObj = m_isSwapped? body1 : body0;
	const btCollider& otherObj = m_isSwapped? body0 : body1;
	btAssert (colObj.getCollisionShape()->isCompound());
	
	const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(colObj.getCollisionShape());

	int numChildren = compoundShape->getNumChildShapes();
	int i;
	
	m_childCollisionAlgorithms.resize(numChildren);
	for (i=0;i<numChildren;i++)
	{
		if (compoundShape->getDynamicAabbTree() || compoundShape->getBvhTree())
		{
			m_childCollisionAlgorithms[i] = 0;
		} else
		{
			const btCollisionShape* tmpShape = colObj.getCollisionShape();
			const btCollisionShape* childShape = compoundShape->getChildShape(i);
			btCollider localCollider(&colObj, childShape, colObj.getCollisionObject(), colObj.getWorldTransform());
			m_childCollisionAlgorithms[i] = dispatcher->findAlgorithm(&localCollider,&otherObj, m_manifoldPtr);
		}
	}
}

void	btCompoundCollisionAlgorithm::removeChildAlgorithms(btDispatcher* dispatcher)
{
	int numChildren = m_childCollisionAlgorithms.size();
	int i;
	for (i=0;i<numChildren;i++)
	{
		if (m_childCollisionAlgorithms[i])
		{
			m_childCollisionAlgorithms[i]->nihilize(dispatcher);
			m_childCollisionAlgorithms[i]->~btCollisionAlgorithm();
			dispatcher->freeCollisionAlgorithm(m_childCollisionAlgorithms[i]);
		}
	}
}

btCompoundCollisionAlgorithm::~btCompoundCollisionAlgorithm()
{
}

void btCompoundCollisionAlgorithm::nihilize(btDispatcher* dispatcher)
{
	removeChildAlgorithms(dispatcher);
	btActivatingCollisionAlgorithm::nihilize(dispatcher);
}


struct	btCompoundLeafCallback : btDbvt::ICollide
{

public:

	const btCollider* m_compoundColObj;
	const btCollider* m_otherObj;
	btDispatcher* m_dispatcher;
	const btDispatcherInfo& m_dispatchInfo;
	btManifoldResult*	m_resultOut;
	btCollisionAlgorithm**	m_childCollisionAlgorithms;
	btPersistentManifold*	m_sharedManifold;




	btCompoundLeafCallback (const btCollider* compoundObj,const btCollider* otherObj,btDispatcher* dispatcher,const btDispatcherInfo& dispatchInfo,btManifoldResult*	resultOut,btCollisionAlgorithm**	childCollisionAlgorithms,btPersistentManifold*	sharedManifold)
		:m_compoundColObj(compoundObj),m_otherObj(otherObj),m_dispatcher(dispatcher),m_dispatchInfo(dispatchInfo),m_resultOut(resultOut),
		m_childCollisionAlgorithms(childCollisionAlgorithms),
		m_sharedManifold(sharedManifold)
	{

	}


	void	ProcessChildShape(const btCollisionShape* childShape,int index)
	{
		btAssert(index>=0);
		const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(m_compoundColObj->getCollisionShape());
		btAssert(index<compoundShape->getNumChildShapes());


		//backup
		btTransform	orgTrans = m_compoundColObj->getWorldTransform();
		const btTransform& childTrans = compoundShape->getChildTransform(index);
		btTransform	newChildWorldTrans = orgTrans*childTrans ;

		//perform an AABB check first
		btVector3 aabbMin0,aabbMax0,aabbMin1,aabbMax1;
		childShape->getAabb(newChildWorldTrans,aabbMin0,aabbMax0);
		m_otherObj->getCollisionShape()->getAabb(m_otherObj->getWorldTransform(),aabbMin1,aabbMax1);

		if (TestAabbAgainstAabb2(aabbMin0,aabbMax0,aabbMin1,aabbMax1))
		{
			//the contactpoint is still projected back using the original inverted worldtrans
			btCollider childCollider(m_compoundColObj, childShape, m_compoundColObj->getCollisionObject(), newChildWorldTrans);

			if (!m_childCollisionAlgorithms[index])
				m_childCollisionAlgorithms[index] = m_dispatcher->findAlgorithm(&childCollider,m_otherObj,m_sharedManifold);

			///detect swapping case
			if (m_resultOut->getBody0Internal() == m_compoundColObj->getCollisionObject())
			{
				m_resultOut->setShapeIdentifiersA(-1,index);
			} else
			{
				m_resultOut->setShapeIdentifiersB(-1,index);
			}
			btCollisionProcessInfo processInfo(childCollider, *m_otherObj, m_dispatchInfo, m_resultOut, m_dispatcher);
			m_childCollisionAlgorithms[index]->processCollision(processInfo);

			if (m_dispatchInfo.m_debugDraw && (m_dispatchInfo.m_debugDraw->getDebugMode() & btIDebugDraw::DBG_DrawAabb))
			{
				btVector3 worldAabbMin,worldAabbMax;
				m_dispatchInfo.m_debugDraw->drawAabb(aabbMin0,aabbMax0,btVector3(1,1,1));
				m_dispatchInfo.m_debugDraw->drawAabb(aabbMin1,aabbMax1,btVector3(1,1,1));
			}
		}
	}
	void		Process(const btDbvtNode* leaf)
	{
		int index = leaf->dataAsInt;

		const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(m_compoundColObj->getCollisionShape());
		const btCollisionShape* childShape = compoundShape->getChildShape(index);
		if (m_dispatchInfo.m_debugDraw && (m_dispatchInfo.m_debugDraw->getDebugMode() & btIDebugDraw::DBG_DrawAabb))
		{
			btVector3 worldAabbMin,worldAabbMax;
			btTransform	orgTrans = m_compoundColObj->getWorldTransform();
			btTransformAabb(leaf->volume.Mins(),leaf->volume.Maxs(),0.,orgTrans,worldAabbMin,worldAabbMax);
			m_dispatchInfo.m_debugDraw->drawAabb(worldAabbMin,worldAabbMax,btVector3(1,0,0));
		}
		ProcessChildShape(childShape,index);

	}
};


struct	MyCompoundNodeOverlapCallback : public btNodeOverlapCallback
{
	const btCollider& m_compoundCollider;
	const btCollider& m_collidingObj;
	btDispatcher* m_dispatcher;
	const btDispatcherInfo& m_dispatchInfo;
	btManifoldResult*	m_resultOut;
	btCollisionAlgorithm**	m_childCollisionAlgorithms;
	btPersistentManifold*	m_sharedManifold;


	MyCompoundNodeOverlapCallback(const btCollider& compoundCollider, 
		const btCollider& collidingObj, 
		btDispatcher* dispatcher,
		const btDispatcherInfo& dispatchInfo,
		btManifoldResult* resultOut,
		btCollisionAlgorithm** childCollisionAlgorithms,
		btPersistentManifold* sharedManifold)
		: m_compoundCollider(compoundCollider)
		, m_collidingObj(collidingObj)
		, m_dispatcher(dispatcher)
		, m_dispatchInfo(dispatchInfo)
		, m_resultOut(resultOut)
		, m_childCollisionAlgorithms(childCollisionAlgorithms)
		, m_sharedManifold(sharedManifold)

	{
	}

	virtual void processNode(int nodeSubPart, int nodeShapeIndex)
	{
		const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(m_compoundCollider.getCollisionShape());
		const btCollisionShape* childShape = compoundShape->getChildShape(nodeShapeIndex);

		//backup
		btTransform	orgTrans = m_compoundCollider.getWorldTransform();
		const btTransform& childTrans = compoundShape->getChildTransform(nodeShapeIndex);
		btTransform	newChildWorldTrans = orgTrans*childTrans ;

		//the contactpoint is still projected back using the original inverted worldtrans
		btCollider childCollider(&m_compoundCollider, childShape,m_compoundCollider.getCollisionObject(), newChildWorldTrans);

		if (!m_childCollisionAlgorithms[nodeShapeIndex])
			m_childCollisionAlgorithms[nodeShapeIndex] = m_dispatcher->findAlgorithm(&childCollider,&m_collidingObj,m_sharedManifold);

		///detect swapping case
		if (m_resultOut->getBody0Internal() == m_compoundCollider.getCollisionObject())
		{
			m_resultOut->setShapeIdentifiersA(-1,nodeShapeIndex);
		} else
		{
			m_resultOut->setShapeIdentifiersB(-1,nodeShapeIndex);
		}
		btCollisionProcessInfo processInfo(childCollider, m_collidingObj, m_dispatchInfo, m_resultOut, m_dispatcher);
		m_childCollisionAlgorithms[nodeShapeIndex]->processCollision(processInfo);
	}
};

void btCompoundCollisionAlgorithm::processCollision (const btCollisionProcessInfo& processInfo)
{
	const btCollider& colObj = m_isSwapped ? processInfo.m_body1 : processInfo.m_body0;
	const btCollider& otherObj = m_isSwapped ? processInfo.m_body0 : processInfo.m_body1;
	const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(colObj.getCollisionShape());

	///btCompoundShape might have changed:
	////make sure the internal child collision algorithm caches are still valid
	if (compoundShape->getUpdateRevision() != m_compoundShapeRevision && !compoundShape->getBvhTree())
	{
		///clear and update all
		removeChildAlgorithms(processInfo.m_dispatcher);
		
		preallocateChildAlgorithms(processInfo.m_dispatcher, processInfo.m_body0, processInfo.m_body1);
	}

	///we need to refresh all contact manifolds
	///note that we should actually recursively traverse all children, btCompoundShape can nested more then 1 level deep
	///so we should add a 'refreshManifolds' in the btCollisionAlgorithm
	{
		for (int i=0; i < m_childCollisionAlgorithms.size(); i++)
		{
			if (m_childCollisionAlgorithms[i])
			{
				m_childCollisionAlgorithms[i]->getAllContactManifolds(m_manifoldArray);
				for (int m = 0; m < m_manifoldArray.size(); m++)
				{
					if (m_manifoldArray[m]->getNumContacts())
					{
						processInfo.m_result->setPersistentManifold(m_manifoldArray[m]);
						processInfo.m_result->refreshContactPoints();
						processInfo.m_result->setPersistentManifold(0);//??necessary?
					}
				}
				m_manifoldArray.resize(0);
			}
		}
	}

	const btDbvt* tree = compoundShape->getDynamicAabbTree();
	const btQuantizedBvh* obvh = compoundShape->getBvhTree();
	if (obvh)
	{
		btVector3 aabbMin, aabbMax;
		MyCompoundNodeOverlapCallback myCallback(colObj, otherObj, processInfo.m_dispatcher, processInfo.m_dispatchInfo, processInfo.m_result, &m_childCollisionAlgorithms[0], m_manifoldPtr);
		btTransform collidingInCompoundSpace = colObj.getWorldTransform().inverse() * otherObj.getWorldTransform();
		otherObj.getCollisionShape()->getAabb(collidingInCompoundSpace, aabbMin, aabbMax);
		// Query bvh-tree here.
		obvh->reportAabbOverlappingNodex(&myCallback,aabbMin,aabbMax);
	}
	else if (tree)
	{
		//use a dynamic aabb tree to cull potential child-overlaps
		btCompoundLeafCallback  callback(
			&colObj,
			&otherObj,
			processInfo.m_dispatcher,
			processInfo.m_dispatchInfo,
			processInfo.m_result,
			&m_childCollisionAlgorithms[0],
			m_manifoldPtr
			);

		btVector3 localAabbMin,localAabbMax;
		btTransform otherInCompoundSpace;
		otherInCompoundSpace = colObj.getWorldTransform().inverse() * otherObj.getWorldTransform();
		otherObj.getCollisionShape()->getAabb(otherInCompoundSpace,localAabbMin,localAabbMax);

		const ATTRIBUTE_ALIGNED16(btDbvtVolume)	bounds=btDbvtVolume::FromMM(localAabbMin,localAabbMax);
		//process all children, that overlap with  the given AABB bounds
		tree->collideTV(tree->m_root,bounds,callback);

	} else
	{
		//iterate over all children, perform an AABB check inside ProcessChildShape
		btCompoundLeafCallback  callback(
			&colObj,
			&otherObj,
			processInfo.m_dispatcher,
			processInfo.m_dispatchInfo,
			processInfo.m_result,
			&m_childCollisionAlgorithms[0],
			m_manifoldPtr
			);
		int numChildren = m_childCollisionAlgorithms.size();
		int i;
		for (i=0;i<numChildren;i++)
		{
			callback.ProcessChildShape(compoundShape->getChildShape(i),i);
		}
	}

	{
				//iterate over all children, perform an AABB check inside ProcessChildShape
		int numChildren = m_childCollisionAlgorithms.size();
		int i;
		btManifoldArray	manifoldArray;
        const btCollisionShape* childShape = 0;
        btTransform	orgTrans;
        btTransform	orgInterpolationTrans;
        btTransform	newChildWorldTrans;
        btVector3 aabbMin0,aabbMax0,aabbMin1,aabbMax1;        
        
		for (i=0;i<numChildren;i++)
		{
			if (m_childCollisionAlgorithms[i])
			{
				childShape = compoundShape->getChildShape(i);
			//if not longer overlapping, remove the algorithm
                orgTrans = colObj.getWorldTransform();
				const btTransform& childTrans = compoundShape->getChildTransform(i);
                newChildWorldTrans = orgTrans*childTrans ;

				//perform an AABB check first
				childShape->getAabb(newChildWorldTrans,aabbMin0,aabbMax0);
				otherObj.getCollisionShape()->getAabb(otherObj.getWorldTransform(),aabbMin1,aabbMax1);

				if (!TestAabbAgainstAabb2(aabbMin0,aabbMax0,aabbMin1,aabbMax1))
				{
					m_childCollisionAlgorithms[i]->nihilize(processInfo.m_dispatcher);
					m_childCollisionAlgorithms[i]->~btCollisionAlgorithm();
					processInfo.m_dispatcher->freeCollisionAlgorithm(m_childCollisionAlgorithms[i]);
					m_childCollisionAlgorithms[i] = 0;
				}
			}
		}
	}
}

btScalar	btCompoundCollisionAlgorithm::calculateTimeOfImpact(btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{

	btCollisionObject* colObj = m_isSwapped? body1 : body0;
	btCollisionObject* otherObj = m_isSwapped? body0 : body1;

	btAssert (colObj->getCollisionShape()->isCompound());
	
	btCompoundShape* compoundShape = static_cast<btCompoundShape*>(colObj->getCollisionShape());

	//We will use the OptimizedBVH, AABB tree to cull potential child-overlaps
	//If both proxies are Compound, we will deal with that directly, by performing sequential/parallel tree traversals
	//given Proxy0 and Proxy1, if both have a tree, Tree0 and Tree1, this means:
	//determine overlapping nodes of Proxy1 using Proxy0 AABB against Tree1
	//then use each overlapping node AABB against Tree0
	//and vise versa.

	btScalar hitFraction = btScalar(1.);

	int numChildren = m_childCollisionAlgorithms.size();
	int i;
    btTransform	orgTrans;
    btScalar frac;
	for (i=0;i<numChildren;i++)
	{
		//temporarily exchange parent btCollisionShape with childShape, and recurse
		btCollisionShape* childShape = compoundShape->getChildShape(i);

		//backup
        orgTrans = colObj->getWorldTransform();
	
		const btTransform& childTrans = compoundShape->getChildTransform(i);
		//btTransform	newChildWorldTrans = orgTrans*childTrans ;
		colObj->setWorldTransform( orgTrans*childTrans );

		btCollisionShape* tmpShape = colObj->getCollisionShape();
		colObj->internalSetTemporaryCollisionShape( childShape );
        frac = m_childCollisionAlgorithms[i]->calculateTimeOfImpact(colObj,otherObj,dispatchInfo,resultOut);
		if (frac<hitFraction)
		{
			hitFraction = frac;
		}
		//revert back
		colObj->internalSetTemporaryCollisionShape( tmpShape);
		colObj->setWorldTransform( orgTrans);
	}
	return hitFraction;

}



struct CompoundConvexCastNodeOverlapCallback : public btNodeOverlapCallback
{
	const btCollider& m_compoundCollider;
	btCollisionWorld::ConvexCastInfo& m_convexCastInfo;

	CompoundConvexCastNodeOverlapCallback(
		const btCollider& compoundCollider,
		btCollisionWorld::ConvexCastInfo& convexCastInfo
		)
		:	m_compoundCollider(compoundCollider)
		,	m_convexCastInfo(convexCastInfo)
	{
	}

	virtual void processNode(int nodeSubPart, int nodeShapeIndex)
	{
		const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(m_compoundCollider.getCollisionShape());
		const btCollisionShape* childShape = compoundShape->getChildShape(nodeShapeIndex);

		const btTransform& orgTrans = m_compoundCollider.getWorldTransform();
		const btTransform& childTrans = compoundShape->getChildTransform(nodeShapeIndex);
		btTransform	newChildWorldTrans = orgTrans * childTrans;

		btCollider childCollider(&m_compoundCollider, childShape, m_compoundCollider.getCollisionObject(), newChildWorldTrans);
		btCollisionWorld::objectQuerySingle(
			m_convexCastInfo.m_castShape,
			m_convexCastInfo.m_convexFromTrans, 
			m_convexCastInfo.m_convexToTrans, 
			childCollider, 
			m_convexCastInfo.m_resultCallback, 
			m_convexCastInfo.m_allowedPenetration
			);
	}
};

void btCompoundCollisionAlgorithm::performConvexcast (
	btCollisionWorld::ConvexCastInfo& convexCastInfo,
	const btCollider& compoundCollider,
	const btVector3& boxSource, 
	const btVector3& boxTarget, 
	const btVector3& aabbMin, 
	const btVector3& aabbMax)
{
	const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(compoundCollider.getCollisionShape());
	const btQuantizedBvh* bvh = compoundShape->getBvhTree();
	CompoundConvexCastNodeOverlapCallback myNodeCallback(compoundCollider, convexCastInfo);
	bvh->reportBoxCastOverlappingNodex (&myNodeCallback, boxSource, boxTarget, aabbMin, aabbMax);
}


