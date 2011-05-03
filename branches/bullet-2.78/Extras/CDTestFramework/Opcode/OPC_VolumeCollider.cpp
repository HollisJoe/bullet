/*
 *	OPCODE - Optimized Collision Detection
 * http://www.codercorner.com/Opcode.htm
 * 
 * Copyright (c) 2001-2008 Pierre Terdiman,  pierre@codercorner.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains base volume collider class.
 *	\file		OPC_VolumeCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains the abstract class for volume colliders.
 *
 *	\class		VolumeCollider
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		June, 2, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VolumeCollider::VolumeCollider() :
	mTouchedPrimitives	(null),
	mNbVolumeBVTests	(0),
	mNbVolumePrimTests	(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VolumeCollider::~VolumeCollider()
{
	mTouchedPrimitives = null;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Validates current settings. You should call this method after all the settings / callbacks have been defined for a collider.
 *	\return		null if everything is ok, else a string describing the problem
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* VolumeCollider::ValidateSettings()
{
	return null;
}

// Pretty dumb way to dump - to do better - one day...

#define IMPLEMENT_NOLEAFDUMP(type)												\
void VolumeCollider::_Dump(const type* node)									\
{																				\
	if(node->HasPosLeaf())	mTouchedPrimitives->Add(node->GetPosPrimitive());	\
	else					_Dump(node->GetPos());								\
																				\
	if(ContactFound()) return;													\
																				\
	if(node->HasNegLeaf())	mTouchedPrimitives->Add(node->GetNegPrimitive());	\
	else					_Dump(node->GetNeg());								\
}

#define IMPLEMENT_LEAFDUMP(type)						\
void VolumeCollider::_Dump(const type* node)			\
{														\
	if(node->IsLeaf())									\
	{													\
		mTouchedPrimitives->Add(node->GetPrimitive());	\
	}													\
	else												\
	{													\
		_Dump(node->GetPos());							\
														\
		if(ContactFound()) return;						\
														\
		_Dump(node->GetNeg());							\
	}													\
}

IMPLEMENT_NOLEAFDUMP(AABBNoLeafNode)
IMPLEMENT_NOLEAFDUMP(AABBQuantizedNoLeafNode)

IMPLEMENT_LEAFDUMP(AABBCollisionNode)
IMPLEMENT_LEAFDUMP(AABBQuantizedNode)
