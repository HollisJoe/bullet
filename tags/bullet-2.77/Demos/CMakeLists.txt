
IF (USE_DX11)
	SUBDIRS(DX11ClothDemo)
ENDIF()

SUBDIRS( HelloWorld )


IF (USE_GLUT)
	IF (GLUT_FOUND)
		IF(BUILD_CPU_DEMOS)
			SET(SharedDemoSubdirs
			OpenGL AllBulletDemos  ConvexDecompositionDemo
			CcdPhysicsDemo ConstraintDemo SliderConstraintDemo GenericJointDemo Raytracer
			RagdollDemo ForkLiftDemo BasicDemo Box2dDemo BspDemo MovingConcaveDemo VehicleDemo
			UserCollisionAlgorithm CharacterDemo SoftDemo HeightFieldFluidDemo
			CollisionInterfaceDemo ConcaveConvexcastDemo SimplexDemo DynamicControlDemo
			DoublePrecisionDemo ConcaveDemo CollisionDemo
			ContinuousConvexCollision ConcaveRaycastDemo GjkConvexCastDemo
			MultiMaterialDemo SerializeDemo InternalEdgeDemo 
			)
		ELSE()
			SET(SharedDemoSubdirs
			OpenGL
		)
		ENDIF()

		SUBDIRS( 		
			${SharedDemoSubdirs}
			Benchmarks
			ThreadingDemo
			MultiThreadedDemo
			VectorAdd_OpenCL
			ParticlesOpenCL
			OpenCLClothDemo
		)
	ENDIF(GLUT_FOUND)
ELSE (USE_GLUT)
	IF (WIN32)
		SUBDIRS( 
		OpenGL 
		BasicDemo 
		Benchmarks 
		Box2dDemo 
		CollisionInterfaceDemo 
		ConcaveDemo 
		ConstraintDemo 
		ConvexDecompositionDemo 
		InternalEdgeDemo
		GimpactTestDemo 
		GenericJointDemo 
		SerializeDemo 
		SoftDemo
		VectorAdd_OpenCL
	)
	ENDIF(WIN32)
ENDIF (USE_GLUT)

