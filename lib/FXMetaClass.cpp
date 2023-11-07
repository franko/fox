/********************************************************************************
*                                                                               *
*                         M e t a C l a s s   O b j e c t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2022 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "fxmath.h"
#include "FXElement.h"
#include "FXArray.h"
#include "FXString.h"
#include "FXMetaClass.h"
#include "FXObject.h"
#include "FXDebugTarget.h"

/*
  Notes:

  - We need a table of all metaclasses, as we should be able to create any type
    of object during deserialization.
  - For MacOS/X support, we moved fxmalloc() and co. here; the reason is that
    when FOX is loaded as a DLL into FXRuby, these symbols need to be resolvable
    in order for the DLL startup code to run properly for the meta class
    initializers; afterward everything's OK.
  - For abstract classes, FXMetaClass contains nullObject() as manufacture function;
    this will always return NULL as abstract classes can't be instantiated.
  - Possibly store hash into FXMetaClass during construction.  Benefits are:
      - No need to recompute it during destruction or growth of hash table.
      - Quick equality test inside getMetaClassFromName().
      - Very minor space penalty.
*/

#define TOPIC_CONSTRUCT 1000

using namespace FX;

namespace FX {

/***********************  Memory Allocation Functions  *************************/

// Allocate memory
FXbool fxmalloc(void** ptr,FXuval size){
  *ptr=nullptr;
  if(size){
    if((*ptr=malloc(size))==nullptr) return false;
    }
  return true;
  }


// Allocate cleaned memory
FXbool fxcalloc(void** ptr,FXuval size){
  *ptr=nullptr;
  if(size){
    if((*ptr=calloc(size,1))==nullptr) return false;
    }
  return true;
  }


// Resize memory
FXbool fxresize(void** ptr,FXuval size){
  void *p=nullptr;
  if(size){
    if((p=realloc(*ptr,size))==nullptr) return false;
    }
  else{
    if(*ptr) free(*ptr);
    }
  *ptr=p;
  return true;
  }


// Allocate and initialize memory
FXbool fxmemdup(void** ptr,const void* src,FXuval size){
  *ptr=nullptr;
  if(src && size){
    if((*ptr=malloc(size))==nullptr) return false;
    memcpy(*ptr,src,size);
    }
  return true;
  }


// String duplicate
FXchar *fxstrdup(const FXchar* str){
  FXchar *ptr=nullptr;
  if(str){
    FXuval size=strlen(str)+1;
    if((ptr=(FXchar*)malloc(size))!=nullptr){
      memcpy(ptr,str,size);
      }
    }
  return ptr;
  }


// Free memory, resets ptr to NULL afterward
void fxfree(void** ptr){
  if(*ptr){
    free(*ptr);
    *ptr=nullptr;
    }
  }


/*************************  FXMetaClass Implementation  ************************/

// Empty but previously used hash table slot
#define EMPTY   ((const FXMetaClass*)-1L)


// Hash table of metaclasses initialized at load-time
const FXMetaClass** FXMetaClass::metaClassTable=nullptr;
FXuint              FXMetaClass::metaClassSlots=0;
FXuint              FXMetaClass::metaClassCount=0;


// Resize global hash table
void FXMetaClass::resize(FXuint slots){
  const FXMetaClass **table;
  const FXMetaClass *ptr;
  FXuint p,x,s;
  callocElms(table,slots);
  for(s=0; s<metaClassSlots; ++s){
    if((ptr=metaClassTable[s])!=nullptr && ptr!=EMPTY){
      p=FXString::hash(ptr->className);
      x=(p<<1)|1;
      while(table[p=(p+x)&(slots-1)]){
        }
      FXASSERT(p<slots);
      table[p]=ptr;
      }
    }
  freeElms(metaClassTable);
  metaClassTable=table;
  metaClassSlots=slots;
  }


// Constructor adds metaclass to the table
FXMetaClass::FXMetaClass(const FXchar* name,FXObject *(fac)(),const FXMetaClass* base,const void* ass,FXuint nass,FXuint assz):className(name),manufacture(fac),baseClass(base),assoc(ass),nassocs(nass),assocsz(assz){
  FXTRACE((TOPIC_CONSTRUCT,"FXMetaClass::FXMetaClass(%s)\n",className));
  FXuint p=FXString::hash(className);
  FXuint x=(p<<1)|1;
  if((++metaClassCount<<1) > metaClassSlots){
    resize(metaClassSlots?metaClassSlots<<1:1);
    }
  FXASSERT(metaClassSlots>=metaClassCount);
  while(metaClassTable[p=(p+x)&(metaClassSlots-1)]){
    }
  FXASSERT(p<metaClassSlots);
  metaClassTable[p]=this;
  }


// Find function
const void* FXMetaClass::search(FXSelector key) const {
  const FXObject::FXMapEntry* lst=(const FXObject::FXMapEntry*)assoc;
  FXuint inc=assocsz;
  FXuint n=nassocs;
  while(n--){
    if(__unlikely(key<=lst->keyhi) && __likely(lst->keylo<=key)) return lst;
    lst=(const FXObject::FXMapEntry*) (((const FXchar*)lst)+inc);
    }
  return nullptr;
  }


// Test if subclass
FXbool FXMetaClass::isSubClassOf(const FXMetaClass* metaclass) const {
  const FXMetaClass* cls;
  for(cls=this; cls; cls=cls->baseClass){
    if(cls==metaclass) return true;
    }
  return false;
  }


// Find the FXMetaClass belonging to class name
const FXMetaClass* FXMetaClass::getMetaClassFromName(const FXchar* name){
  if(name && name[0] && metaClassSlots){
    FXuint p=FXString::hash(name);
    FXuint x=(p<<1)|1;
    while(metaClassTable[p=(p+x)&(metaClassSlots-1)]){
      if(metaClassTable[p]!=EMPTY && FXString::compare(metaClassTable[p]->className,name)==0){
        return metaClassTable[p];
        }
      }
    }
  return nullptr;
  }


// Find the FXMetaClass belonging to class name
const FXMetaClass* FXMetaClass::getMetaClassFromName(const FXString& name){
  return getMetaClassFromName(name.text());
  }


// Make instance of class name, a subclass of a given base class
FXObject* FXMetaClass::makeInstanceOfName(const FXchar* name){
  const FXMetaClass *metaclass=getMetaClassFromName(name);
  if(metaclass){
    return metaclass->makeInstance();
    }
  return nullptr;
  }


// Make instance of class name, a subclass of a given base class
FXObject* FXMetaClass::makeInstanceOfName(const FXString& name){
  return makeInstanceOfName(name.text());
  }


// Create an object instance
FXObject* FXMetaClass::makeInstance() const {
  return (*manufacture)();
  }


// Make NULL object; used for abstract classes that may not be instantiated
FXObject* FXMetaClass::nullObject(){
  return nullptr;
  }


// Destructor removes metaclass from the table
FXMetaClass::~FXMetaClass(){
  FXTRACE((TOPIC_CONSTRUCT,"FXMetaClass::~FXMetaClass(%s)\n",className));
  FXuint p=FXString::hash(className);
  FXuint x=(p<<1)|1;
  while(metaClassTable[p=(p+x)&(metaClassSlots-1)]!=this){
    if(!metaClassTable[p]) return;
    }
  metaClassTable[p]=EMPTY;
  if((--metaClassCount<<1) <= metaClassSlots){
    resize(metaClassSlots>>1);
    }
  FXASSERT(metaClassSlots>=metaClassCount);
  }


void FXMetaClass::dumpMessageMap(const FXMetaClass* m){
  fxmessage("FXMetaClass:%s:%s\n",m->className,m->baseClass?m->baseClass->className:"");
  FXuint i=0;
  do{
    const FXObject::FXMapEntry* map=(const FXObject::FXMapEntry*)m->assoc;
    FXuint inc=m->assocsz;
    FXuint n=m->nassocs;
    while(n--){
      fxmessage("%4u:   %30s:%-5u...%30s:%-5u\n",i,FXDebugTarget::messageTypeName[FXSELTYPE(map->keylo)],FXSELID(map->keylo),FXDebugTarget::messageTypeName[FXSELTYPE(map->keyhi)],FXSELID(map->keyhi));
      map=(const FXObject::FXMapEntry*) (((const FXchar*)map)+inc);
      i++;
      }
    m=m->baseClass;
    fxmessage("\n");
    }
  while(m);
  fxmessage("\n");
  }


void FXMetaClass::dumpMetaClasses(){
  for(FXint m=0; m<metaClassCount; ++m){
    if(metaClassTable[m]==nullptr || metaClassTable[m]==EMPTY) continue;
    dumpMessageMap(metaClassTable[m]);
    }
  }

}
