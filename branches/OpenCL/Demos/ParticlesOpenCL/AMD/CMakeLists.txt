

INCLUDE_DIRECTORIES(
${BULLET_PHYSICS_SOURCE_DIR}/src 
${BULLET_PHYSICS_SOURCE_DIR}/Extras/OpenCL/Shared
${BULLET_PHYSICS_SOURCE_DIR}/Demos/OpenGL
)

IF (INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
	INCLUDE_DIRECTORIES(		$ENV{==AMDAPPSDKROOT=}/include )
	IF (CMAKE_CL_64)
		SET(CMAK_AMDAPPSDKROOT_LIBPATH 		$ENV{==AMDAPPSDKROOT=}/lib/x86_64		)
	ELSE(CMAKE_CL_64)
		SET(CMAK_AMDAPPSDKROOT_LIBPATH		$ENV{==AMDAPPSDKROOT=}/lib/x86		)
	ENDIF(CMAKE_CL_64)
ELSE()
	INCLUDE_DIRECTORIES(		$ENV{AMDAPPSDKROOT}/include	)
	IF (CMAKE_CL_64)
		SET(CMAK_AMDAPPSDKROOT_LIBPATH 		$ENV{AMDAPPSDKROOT}/lib/x86_64 )
	ELSE(CMAKE_CL_64)
		SET(CMAK_AMDAPPSDKROOT_LIBPATH		$ENV{AMDAPPSDKROOT}/lib/x86		)
	ENDIF(CMAKE_CL_64)
ENDIF()


IF (CMAKE_CL_64)
	SET(CMAK_GLEW_LIBRARY
		${BULLET_PHYSICS_SOURCE_DIR}/Glut/glew64.lib		)
ELSE(CMAKE_CL_64)
	SET(CMAK_GLEW_LIBRARY		${BULLET_PHYSICS_SOURCE_DIR}/Glut/glew32.lib		)
ENDIF(CMAKE_CL_64)


IF (USE_GLUT)
	LINK_LIBRARIES(
		OpenGLSupport 
		BulletDynamics  
		BulletCollision 
		BulletMultiThreaded 
		LinearMath 
		${GLUT_glut_LIBRARY} 
		${OPENGL_gl_LIBRARY} 
		${OPENGL_glu_LIBRARY}
		${CMAK_GLEW_LIBRARY}
		${CMAK_AMDAPPSDKROOT_LIBPATH}/OpenCL.lib
	)


	ADD_EXECUTABLE(AppParticlesOCL_AMD
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/btParticlesDynamicsWorld.h
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/btParticlesSharedDefs.h
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/btParticlesSharedTypes.h
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/ParticlesDemo.h
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/btOclUtils.h
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/shaders.h	
					${BULLET_PHYSICS_SOURCE_DIR}/Extras/OpenCL/Shared/btOclCommon.h
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/btOclUtils.cpp
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/btParticlesDemoDynamicsWorld.cpp
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/main.cpp
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/ParticlesDemo.cpp
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/shaders.cpp
					${BULLET_PHYSICS_SOURCE_DIR}/Extras/OpenCL/Shared/btOclCommon.cpp
					${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/ParticlesOCL.cl
	)
ELSE (USE_GLUT)
ENDIF (USE_GLUT)

IF(WIN32)
IF (CMAKE_CL_64)
	IF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
		ADD_CUSTOM_COMMAND(	TARGET AppParticlesOCL_AMD		POST_BUILD
					COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/glut64.dll ${CMAKE_CURRENT_BINARY_DIR}	)
		ADD_CUSTOM_COMMAND(	TARGET AppParticlesOCL_AMD	POST_BUILD
					COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/GLEW64.DLL ${CMAKE_CURRENT_BINARY_DIR})
		ADD_CUSTOM_COMMAND(	TARGET AppParticlesOCL_AMD	POST_BUILD
					COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/ParticlesOCL.cl ${CMAKE_CURRENT_BINARY_DIR})
	ENDIF()
ELSE(CMAKE_CL_64)
	IF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
		ADD_CUSTOM_COMMAND(	TARGET AppParticlesOCL_AMD	POST_BUILD
					COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/GLUT32.DLL ${CMAKE_CURRENT_BINARY_DIR})
		ADD_CUSTOM_COMMAND(	TARGET AppParticlesOCL_AMD	POST_BUILD
					COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/GLEW32.DLL ${CMAKE_CURRENT_BINARY_DIR})
		ADD_CUSTOM_COMMAND(	TARGET AppParticlesOCL_AMD	POST_BUILD
					COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Demos/ParticlesOpenCL/ParticlesOCL.cl ${CMAKE_CURRENT_BINARY_DIR})
	ENDIF()
ENDIF(CMAKE_CL_64)
ENDIF(WIN32)

IF (UNIX)
  TARGET_LINK_LIBRARIES(AppParticlesOCL_AMD pthread)
ENDIF(UNIX)

