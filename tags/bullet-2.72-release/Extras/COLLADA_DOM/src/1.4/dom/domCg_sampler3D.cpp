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
#include <dom/domCg_sampler3D.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domCg_sampler3D::create(daeInt bytes)
{
	domCg_sampler3DRef ref = new(bytes) domCg_sampler3D;
	return ref;
}


daeMetaElement *
domCg_sampler3D::registerElement()
{
    if ( _Meta != NULL ) return _Meta;
    
    _Meta = new daeMetaElement;
    _Meta->setName( "cg_sampler3D" );
	_Meta->registerClass(domCg_sampler3D::create, &_Meta);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( _Meta, cm, 0, 1, 1 );

	cm = new daeMetaSequence( _Meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( _Meta, cm, 0, 1, 1 );
	mea->setName( "source" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemSource) );
	mea->setElementType( domSource::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 1, 0, 1 );
	mea->setName( "wrap_s" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemWrap_s) );
	mea->setElementType( domWrap_s::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 2, 0, 1 );
	mea->setName( "wrap_t" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemWrap_t) );
	mea->setElementType( domWrap_t::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 3, 0, 1 );
	mea->setName( "wrap_p" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemWrap_p) );
	mea->setElementType( domWrap_p::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 4, 0, 1 );
	mea->setName( "minfilter" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemMinfilter) );
	mea->setElementType( domMinfilter::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 5, 0, 1 );
	mea->setName( "magfilter" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemMagfilter) );
	mea->setElementType( domMagfilter::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 6, 0, 1 );
	mea->setName( "mipfilter" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemMipfilter) );
	mea->setElementType( domMipfilter::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 7, 0, 1 );
	mea->setName( "border_color" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemBorder_color) );
	mea->setElementType( domBorder_color::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 8, 0, 1 );
	mea->setName( "mipmap_maxlevel" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemMipmap_maxlevel) );
	mea->setElementType( domMipmap_maxlevel::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementAttribute( _Meta, cm, 9, 0, 1 );
	mea->setName( "mipmap_bias" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemMipmap_bias) );
	mea->setElementType( domMipmap_bias::registerElement() );
	cm->appendChild( mea );
	
	mea = new daeMetaElementArrayAttribute( _Meta, cm, 10, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domCg_sampler3D,elemExtra_array) );
	mea->setElementType( domExtra::registerElement() );
	cm->appendChild( mea );
	
	cm->setMaxOrdinal( 10 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();
	
	cm->setMaxOrdinal( 10 );
	_Meta->setCMRoot( cm );	
	
	
	_Meta->setElementSize(sizeof(domCg_sampler3D));
	_Meta->validate();

	return _Meta;
}


daeMetaElement * domCg_sampler3D::_Meta = NULL;


