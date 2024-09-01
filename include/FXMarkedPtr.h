/********************************************************************************
*                                                                               *
*                    M a r k e d   P o i n t e r   C l a s s                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2023,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#ifndef FXMARKEDPTR_H
#define FXMARKEDPTR_H

namespace FX {


/**
* Marked pointer keeps a flag bit inside of a dynamically allocated pointer;
* this is possible because memory returned by an allocator is aligned to a
* multiple of the processor's "natural type", typically a multiple of 4-, 8-,
* or even 16-bytes.
*/
template<typename TYPE>
class FXMarkedPtr {
private:
  union Un { FXuval val; TYPE* ptr; };
private:
  Un as;
public:

  // Default constructor
  FXMarkedPtr(){ as.ptr=nullptr; }

  // Init with pointer
  FXMarkedPtr(TYPE* p){ as.ptr=p; }

  // Copy constructor
  FXMarkedPtr(const FXMarkedPtr<TYPE>& org):as(org.as){ }

  // Assignment
  FXMarkedPtr<TYPE>& operator=(TYPE* p){ as.ptr=p; return *this; }

  // Assignment
  FXMarkedPtr<TYPE>& operator=(const FXMarkedPtr<TYPE>& org){ as=org.as; return *this; }

  // Obtain the pointer part, stripping off the flag
  TYPE* ptr() const { Un x={as.val&~1L}; return x.ptr; }

  // Conversion operators
  operator TYPE*() const { return ptr(); }

  // Dereference operator
  TYPE& operator*() const { return *ptr(); }

  // Follow pointer operator
  TYPE* operator->() const { return ptr(); }

  // Test for non-null
  operator FXbool() const { return !!ptr(); }

  // Test for NULL
  FXbool operator!() const { return !ptr(); }

  // Comparison operator.
  FXbool operator==(TYPE *p) const { return ptr()==p; }

  // Comparison operator.
  FXbool operator!=(TYPE *p) const { return ptr()!=p; }

  // Get flag
  FXbool flag() const { return !!(as.val&1L); }

  // Set flag
  void flag(FXbool flg){ as.val^=((0-flg)^as.val)&1L; }

  // Flip flag
  void flip(){ as.val^=1L; }

  // Destroy
 ~FXMarkedPtr(){ }
  };

}

#endif
