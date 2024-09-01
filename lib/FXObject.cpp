/********************************************************************************
*                                                                               *
*                         T o p l e v e l   O b j e c t                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXElement.h"
#include "FXArray.h"
#include "FXMetaClass.h"
#include "FXObject.h"
#include "FXException.h"


/*
  Notes:

  - FXObject is the top of the class hierarchy.  Metaclass for FXObject can
    not be made by the macros, so do it by hand.
  - Message dispatch using tryHandle() catches all resource exceptions; this
    is so that if such exceptions are thrown, resources can be reclaimed during
    the unroll to the widget calling tryHandle, at which point we can proceed
    normally since this was the last-known-good point.
    Thus a simple mechanism is created whereby properly written FOX programs
    can recover gracefully from resource limitation driven exceptions.
*/

using namespace FX;

namespace FX {

/*******************************************************************************/

/// EXPERIMENT ///
#if defined(NEWMAP)



static const NewMapEntry messagemap[]={
//  {100,200,&method_call<&FXObject::onDefault>},
  {100,200,&FXMessageCallback::MethodCall<FXObject,&FXObject::onDefault>},
  };

/*
if you define a function template

  template <typename t, void (t::*mf)()> foo<t, mf> f(mf);

where foo is your old-style class template like

  template <typename t, void (t::*mf)()> class foo;

then you can use decltype(f(&x::h)) to deduce the desired type foo<x, &x::h> without having to repeat x.
the price is that you either need to say decltype everywhere, or you wrap that in a macro.

*/

#endif
/// EXPERIMENT ///


// Have to do this one `by hand' as it has no base class
const FXMetaClass FXObject::metaClass("FXObject",FXObject::manufacture,nullptr,nullptr,0,0);


// Build an object
FXObject* FXObject::manufacture(){
  return new FXObject;
  }


// Get metaclass of object
const FX::FXMetaClass* FXObject::getMetaClass() const {
  return &FXObject::metaClass;
  }


// Get class name of object
const FXchar* FXObject::getClassName() const {
  return getMetaClass()->getClassName();
  }


// Check if object belongs to a class
FXbool FXObject::isMemberOf(const FXMetaClass* metaclass) const {
  return getMetaClass()->isSubClassOf(metaclass);
  }


// Unhandled function
long FXObject::onDefault(FXObject*,FXSelector,void*){
  return 0;
  }


// Handle message
long FXObject::handle(FXObject* sender,FXSelector sel,void* ptr){
  return onDefault(sender,sel,ptr);
  }


// Try handle message safely; we catch only resource exceptions, things like
// running out of memory, window handles, system resources; others are ignored.
long FXObject::tryHandle(FXObject* sender,FXSelector sel,void* ptr){
  try { return handle(sender,sel,ptr); } catch(const FXResourceException&) { return 0; }
  }


// Save to stream
void FXObject::save(FXStream&) const { }


// Load from stream
void FXObject::load(FXStream&){ }


// Virtual destructor
FXObject::~FXObject(){
  }

}
