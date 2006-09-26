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

#ifndef PERSISTENT_MANIFOLD_H
#define PERSISTENT_MANIFOLD_H


#include "LinearMath/SimdVector3.h"
#include "LinearMath/SimdTransform.h"
#include "btManifoldPoint.h"

struct btCollisionResult;

///contact breaking and merging treshold
extern float gContactBreakingTreshold;

typedef bool (*ContactDestroyedCallback)(void* userPersistentData);
extern ContactDestroyedCallback	gContactDestroyedCallback;




#define MANIFOLD_CACHE_SIZE 4

///PersistentManifold maintains contact points, and reduces them to 4.
///It does contact filtering/contact reduction.
class btPersistentManifold 
{

	btManifoldPoint m_pointCache[MANIFOLD_CACHE_SIZE];

	/// this two body pointers can point to the physics rigidbody class.
	/// void* will allow any rigidbody class
	void* m_body0;
	void* m_body1;
	int	m_cachedPoints;

	
	/// sort cached points so most isolated points come first
	int	SortCachedPoints(const btManifoldPoint& pt);

	int		FindContactPoint(const btManifoldPoint* unUsed, int numUnused,const btManifoldPoint& pt);

public:

	int m_index1;

	btPersistentManifold();

	btPersistentManifold(void* body0,void* body1)
		: m_body0(body0),m_body1(body1),m_cachedPoints(0)
	{
	}

	inline void* GetBody0() { return m_body0;}
	inline void* GetBody1() { return m_body1;}

	inline const void* GetBody0() const { return m_body0;}
	inline const void* GetBody1() const { return m_body1;}

	void	SetBodies(void* body0,void* body1)
	{
		m_body0 = body0;
		m_body1 = body1;
	}

	void ClearUserCache(btManifoldPoint& pt);

#ifdef DEBUG_PERSISTENCY
	void	DebugPersistency();
#endif //
	
	inline int	GetNumContacts() const { return m_cachedPoints;}

	inline const btManifoldPoint& GetContactPoint(int index) const
	{
		ASSERT(index < m_cachedPoints);
		return m_pointCache[index];
	}

	inline btManifoldPoint& GetContactPoint(int index)
	{
		ASSERT(index < m_cachedPoints);
		return m_pointCache[index];
	}

	/// todo: get this margin from the current physics / collision environment
	float	GetContactBreakingTreshold() const;
	
	int GetCacheEntry(const btManifoldPoint& newPoint) const;

	void AddManifoldPoint( const btManifoldPoint& newPoint);

	void RemoveContactPoint (int index)
	{
		ClearUserCache(m_pointCache[index]);

		int lastUsedIndex = GetNumContacts() - 1;
		m_pointCache[index] = m_pointCache[lastUsedIndex];
		//get rid of duplicated userPersistentData pointer
		m_pointCache[lastUsedIndex].m_userPersistentData = 0;
		m_cachedPoints--;
	}
	void ReplaceContactPoint(const btManifoldPoint& newPoint,int insertIndex)
	{
		assert(ValidContactDistance(newPoint));

		ClearUserCache(m_pointCache[insertIndex]);
		
		m_pointCache[insertIndex] = newPoint;
	}

	bool ValidContactDistance(const btManifoldPoint& pt) const
	{
		return pt.m_distance1 <= GetContactBreakingTreshold();
	}
	/// calculated new worldspace coordinates and depth, and reject points that exceed the collision margin
	void	RefreshContactPoints(  const btSimdTransform& trA,const btSimdTransform& trB);

	void	ClearManifold();



};



#endif //PERSISTENT_MANIFOLD_H
