/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License 
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
 * implied. See the License for the specific language governing permissions and limitations under the 
 * License. 
 */

#include <dae/daeDom.h>
#include <dom/domInstance_rigid_constraint.h>

daeElementRef
domInstance_rigid_constraint::create(daeInt bytes)
{
	domInstance_rigid_constraintRef ref = new(bytes) domInstance_rigid_constraint;
	return ref;
}


daeMetaElement *
domInstance_rigid_constraint::registerElement()
{
    if ( _Meta != NULL ) return _Meta;
    
    _Meta = new daeMetaElement;
    _Meta->setName( "instance_rigid_constraint" );
	_Meta->setStaticPointerAddress(&domInstance_rigid_constraint::_Meta);
	_Meta->registerConstructor(domInstance_rigid_constraint::create);

	// Add elements: extra
    _Meta->appendArrayElement(domExtra::registerElement(),daeOffsetOf(domInstance_rigid_constraint,elemExtra_array));

	//	Add attribute: constraint
 	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "constraint" );
		ma->setType( daeAtomicType::get("xsNCName"));
		ma->setOffset( daeOffsetOf( domInstance_rigid_constraint , attrConstraint ));
		ma->setContainer( _Meta );
		ma->setIsRequired( true );
	
		_Meta->appendAttribute(ma);
	}
	
	
	_Meta->setElementSize(sizeof(domInstance_rigid_constraint));
	_Meta->validate();

	return _Meta;
}


daeMetaElement * domInstance_rigid_constraint::_Meta = NULL;


