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

/*
	SimplexDemo demonstrated the working of the subdistance algorithm as used in GJK.
	It draws the simplex, and calculates the closest vector from simplex to the origin
*/

#include "GL_Simplex1to4.h"
#include "SimdQuaternion.h"
#include "SimdTransform.h"
#include "GL_ShapeDrawer.h"

#include "NarrowPhaseCollision/VoronoiSimplexSolver.h"
#include "SimplexDemo.h"
#include "GlutStuff.h"

VoronoiSimplexSolver	simplexSolver;



float yaw=0.f,pitch=0.f,roll=0.f;
const int maxNumObjects = 4;
const int numObjects = 1;
int screenWidth = 640;
int screenHeight = 480;
/// simplex contains the vertices, and some extra code to draw and debug
GL_Simplex1to4	simplex;


PolyhedralConvexShape*	shapePtr[maxNumObjects];


///
///
///
int main(int argc,char** argv)
{

	SimplexDemo* demo = new SimplexDemo();

	demo->initPhysics();
	
	return glutmain(argc, argv,screenWidth,screenHeight,"SimplexDemo",demo);
}

//to be implemented by the demo

void SimplexDemo::clientMoveAndDisplay()
{
	
	displayCallback();
}



void SimplexDemo::displayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glDisable(GL_LIGHTING);

	GL_ShapeDrawer::DrawCoordSystem();

	float m[16];
	int i;

	for (i=0;i<numObjects;i++)
	{
		SimdTransform transA;
		transA.setIdentity();
		SimdVector3	dpos(0.f,5.f,0.f);
		transA.setOrigin( dpos );
		SimdQuaternion orn;
		orn.setEuler(yaw,pitch,roll);
		transA.setRotation(orn);
		transA.getOpenGLMatrix( m );

		/// draw the simplex
		GL_ShapeDrawer::DrawOpenGL(m,shapePtr[i],SimdVector3(1,1,1),getDebugMode());

		/// calculate closest point from simplex to the origin, and draw this vector
		simplex.CalcClosest(m);

	}
	pitch += 0.005f;
	yaw += 0.01f;

	glFlush();
    glutSwapBuffers();
}

void	SimplexDemo::initPhysics()
{

	simplex.SetSimplexSolver(&simplexSolver);

	simplex.AddVertex(SimdPoint3(-2,0,-2));
	simplex.AddVertex(SimdPoint3(2,0,-2));
	simplex.AddVertex(SimdPoint3(0,0,2));
	simplex.AddVertex(SimdPoint3(0,2,0));

	shapePtr[0] = &simplex;

	SimdTransform tr;
	tr.setIdentity();
}


