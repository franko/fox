/********************************************************************************
*                                                                               *
*                         M e t a C l a s s   O b j e c t                       *
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
#ifndef FXMETACLASS_H
#define FXMETACLASS_H

namespace FX {


class FXObject;

/// Association key
typedef FXuint FXSelector;


/// Describes a FOX object
class FXAPI FXMetaClass {
private:
  const FXchar       *className;        // Class name
  FXObject*         (*manufacture)();   // Factory function
  const FXMetaClass  *baseClass;        // Base classes' metaclass
  const void         *assoc;            // Associated handlers
  FXuint              nassocs;          // Count of handlers
  FXuint              assocsz;          // Size of association
private:
  static const FXMetaClass **metaClassTable;    // Class table
  static FXuint              metaClassSlots;    // Number of slots
  static FXuint              metaClassCount;    // Number items
private:
  static void resize(FXuint slots);
public:
  static void dumpMessageMap(const FXMetaClass* m);
  static void dumpMetaClasses();
private:
  FXMetaClass(const FXMetaClass&);
  FXMetaClass &operator=(const FXMetaClass&);
public:

  /// Create one metaclass for each class
  FXMetaClass(const FXchar* name,FXObject *(fac)(),const FXMetaClass* base,const void* ass,FXuint nass,FXuint assz);

  /// Search message map
  const void* search(FXSelector key) const;

  /// Ask class name
  const FXchar* getClassName() const { return className; }

  /// Ask base class
  const FXMetaClass* getBaseClass() const { return baseClass; }

  /// Check if metaclass is subclass of some other metaclass
  FXbool isSubClassOf(const FXMetaClass* metaclass) const;

  /// Find metaclass object
  static const FXMetaClass* getMetaClassFromName(const FXchar* name);
  static const FXMetaClass* getMetaClassFromName(const FXString& name);

  /// Make instance of class name, a subclass of a given base class
  static FXObject* makeInstanceOfName(const FXchar* name);
  static FXObject* makeInstanceOfName(const FXString& name);

  /// Make instance of some object
  FXObject* makeInstance() const;

  /// Make NULL object
  static FXObject* nullObject();

  /// Destroy metaclass
  virtual ~FXMetaClass();
  };

}

#endif
