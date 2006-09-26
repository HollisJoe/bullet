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

#include "btStaticPlaneShape.h"

#include "LinearMath/SimdTransformUtil.h"


btStaticPlaneShape::btStaticPlaneShape(const btSimdVector3& planeNormal,SimdScalar planeConstant)
:m_planeNormal(planeNormal),
m_planeConstant(planeConstant),
m_localScaling(0.f,0.f,0.f)
{
}


btStaticPlaneShape::~btStaticPlaneShape()
{
}



void btStaticPlaneShape::GetAabb(const btSimdTransform& t,btSimdVector3& aabbMin,btSimdVector3& aabbMax) const
{
	btSimdVector3 infvec (1e30f,1e30f,1e30f);

	btSimdVector3 center = m_planeNormal*m_planeConstant;
	aabbMin = center + infvec*m_planeNormal;
	aabbMax = aabbMin;
	aabbMin.setMin(center - infvec*m_planeNormal);
	aabbMax.setMax(center - infvec*m_planeNormal); 

	aabbMin.setValue(-1e30f,-1e30f,-1e30f);
	aabbMax.setValue(1e30f,1e30f,1e30f);

}




void	btStaticPlaneShape::ProcessAllTriangles(btTriangleCallback* callback,const btSimdVector3& aabbMin,const btSimdVector3& aabbMax) const
{

	btSimdVector3 halfExtents = (aabbMax - aabbMin) * 0.5f;
	SimdScalar radius = halfExtents.length();
	btSimdVector3 center = (aabbMax + aabbMin) * 0.5f;
	
	//this is where the triangles are generated, given AABB and plane equation (normal/constant)

	btSimdVector3 tangentDir0,tangentDir1;

	//tangentDir0/tangentDir1 can be precalculated
	SimdPlaneSpace1(m_planeNormal,tangentDir0,tangentDir1);

	btSimdVector3 supVertex0,supVertex1;

	btSimdVector3 projectedCenter = center - (m_planeNormal.dot(center) - m_planeConstant)*m_planeNormal;
	
	btSimdVector3 triangle[3];
	triangle[0] = projectedCenter + tangentDir0*radius + tangentDir1*radius;
	triangle[1] = projectedCenter + tangentDir0*radius - tangentDir1*radius;
	triangle[2] = projectedCenter - tangentDir0*radius - tangentDir1*radius;

	callback->ProcessTriangle(triangle,0,0);

	triangle[0] = projectedCenter - tangentDir0*radius - tangentDir1*radius;
	triangle[1] = projectedCenter - tangentDir0*radius + tangentDir1*radius;
	triangle[2] = projectedCenter + tangentDir0*radius + tangentDir1*radius;

	callback->ProcessTriangle(triangle,0,1);

}

void	btStaticPlaneShape::CalculateLocalInertia(SimdScalar mass,btSimdVector3& inertia)
{
	//moving concave objects not supported
	
	inertia.setValue(0.f,0.f,0.f);
}

void	btStaticPlaneShape::setLocalScaling(const btSimdVector3& scaling)
{
	m_localScaling = scaling;
}
const btSimdVector3& btStaticPlaneShape::getLocalScaling() const
{
	return m_localScaling;
}
