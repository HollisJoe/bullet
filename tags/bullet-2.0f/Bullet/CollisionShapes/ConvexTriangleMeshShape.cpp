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
#include "ConvexTriangleMeshShape.h"
#include "CollisionShapes/CollisionMargin.h"

#include "SimdQuaternion.h"
#include "CollisionShapes/StridingMeshInterface.h"


ConvexTriangleMeshShape ::ConvexTriangleMeshShape (StridingMeshInterface* meshInterface)
:m_stridingMesh(meshInterface)
{
}




///It's not nice to have all this virtual function overhead, so perhaps we can also gather the points once
///but then we are duplicating
class LocalSupportVertexCallback: public InternalTriangleIndexCallback
{

	SimdVector3 m_supportVertexLocal;
public:

	SimdScalar m_maxDot;
	SimdVector3 m_supportVecLocal;

	LocalSupportVertexCallback(const SimdVector3& supportVecLocal)
		: m_supportVertexLocal(0.f,0.f,0.f),
		m_maxDot(-1e30f),
                m_supportVecLocal(supportVecLocal)
	{
	}

	virtual void InternalProcessTriangleIndex(SimdVector3* triangle,int partId,int  triangleIndex)
	{
		for (int i=0;i<3;i++)
		{
			SimdScalar dot = m_supportVecLocal.dot(triangle[i]);
			if (dot > m_maxDot)
			{
				m_maxDot = dot;
				m_supportVertexLocal = triangle[i];
			}
		}
	}
	
	SimdVector3	GetSupportVertexLocal()
	{
		return m_supportVertexLocal;
	}

};





SimdVector3	ConvexTriangleMeshShape::LocalGetSupportingVertexWithoutMargin(const SimdVector3& vec0)const
{
	SimdVector3 supVec(0.f,0.f,0.f);

	SimdVector3 vec = vec0;
	SimdScalar lenSqr = vec.length2();
	if (lenSqr < 0.0001f)
	{
		vec.setValue(1,0,0);
	} else
	{
		float rlen = 1.f / SimdSqrt(lenSqr );
		vec *= rlen;
	}

	LocalSupportVertexCallback	supportCallback(vec);
	SimdVector3 aabbMax(1e30f,1e30f,1e30f);
	m_stridingMesh->InternalProcessAllTriangles(&supportCallback,-aabbMax,aabbMax);
	supVec = supportCallback.GetSupportVertexLocal();

	return supVec;
}

void	ConvexTriangleMeshShape::BatchedUnitVectorGetSupportingVertexWithoutMargin(const SimdVector3* vectors,SimdVector3* supportVerticesOut,int numVectors) const
{
	//use 'w' component of supportVerticesOut?
	{
		for (int i=0;i<numVectors;i++)
		{
			supportVerticesOut[i][3] = -1e30f;
		}
	}
	
	//todo: could do the batch inside the callback!


	for (int j=0;j<numVectors;j++)
	{
		const SimdVector3& vec = vectors[j];
		LocalSupportVertexCallback	supportCallback(vec);
		SimdVector3 aabbMax(1e30f,1e30f,1e30f);
		m_stridingMesh->InternalProcessAllTriangles(&supportCallback,-aabbMax,aabbMax);
		supportVerticesOut[j] = supportCallback.GetSupportVertexLocal();
	}
	
}
	


SimdVector3	ConvexTriangleMeshShape::LocalGetSupportingVertex(const SimdVector3& vec)const
{
	SimdVector3 supVertex = LocalGetSupportingVertexWithoutMargin(vec);

	if ( GetMargin()!=0.f )
	{
		SimdVector3 vecnorm = vec;
		if (vecnorm .length2() < (SIMD_EPSILON*SIMD_EPSILON))
		{
			vecnorm.setValue(-1.f,-1.f,-1.f);
		} 
		vecnorm.normalize();
		supVertex+= GetMargin() * vecnorm;
	}
	return supVertex;
}









//currently just for debugging (drawing), perhaps future support for algebraic continuous collision detection
//Please note that you can debug-draw ConvexTriangleMeshShape with the Raytracer Demo
int	ConvexTriangleMeshShape::GetNumVertices() const
{
	//cache this?
	assert(0);
	return 0;
	
}

int ConvexTriangleMeshShape::GetNumEdges() const
{
	assert(0);	
	return 0;
}

void ConvexTriangleMeshShape::GetEdge(int i,SimdPoint3& pa,SimdPoint3& pb) const
{
	assert(0);	
}

void ConvexTriangleMeshShape::GetVertex(int i,SimdPoint3& vtx) const
{
	assert(0);
}

int	ConvexTriangleMeshShape::GetNumPlanes() const
{
	return 0;
}

void ConvexTriangleMeshShape::GetPlane(SimdVector3& planeNormal,SimdPoint3& planeSupport,int i ) const
{
	assert(0);
}

//not yet
bool ConvexTriangleMeshShape::IsInside(const SimdPoint3& pt,SimdScalar tolerance) const
{
	assert(0);
	return false;
}



void	ConvexTriangleMeshShape::setLocalScaling(const SimdVector3& scaling)
{
	m_stridingMesh->setScaling(scaling);
}

