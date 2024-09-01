/********************************************************************************
*                                                                               *
*                         F O X   E v e n t   L o o p                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 2019,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "FXEventLoop.h"

/*
  Notes:

  - FXEventLoop supports construction of nested event loops, for instance for
    dialogs or popups.

  - The FXEventLoops are linked, so that outer nested loops may be found.  This
    allows setting of termination conditions and return codes for the whole stack,
    even from a deeply nested FXEventLoop.

  - The invocation member variable points to the variable holding a pointer to the
    currently inner-most FXEventLoop.

  - In the FXEventLoop constructor, the value of *invocation is copied into the
    upper member variable, and then set to point to the newly created instance.

  - In FXEventLoop destructor, *invocation is replaced by the outer FXEventloop,
    which was saved in the upper member variable.

  - Having FXEventLoop manage the linked list of FXEventLoops this way means
    any exceptions thrown will nicely unroll and clean up the FXEventLoop
    instances until reaching a place where the exceptions are handled.

  - There are several loop-termination functions:

      stop()            Sets done-flag on all loops in the linked list.

      stopModal(v)      Sets done-flag on all loops below the modal one.

      stopModal(win,v)  Sets done-flag on all loops below the modal-for one.

  -

*/

using namespace FX;

/*******************************************************************************/

namespace FX {


// Enter modal loop
FXEventLoop::FXEventLoop(FXEventLoop** inv,FXWindow* win,FXuint mode):invocation(inv),upper(*invocation),modalWindow(win),modalType(mode),code(0),exit(false){
  *invocation=this;
  }


// Test if the window is involved in a modal invocation
FXbool FXEventLoop::isModal() const {
  for(const FXEventLoop* eventloop=this; eventloop; eventloop=eventloop->upper){
    if(eventloop->modalType) return true;
    }
  return false;
  }


// Test if the window is involved in a modal invocation
FXbool FXEventLoop::isModal(FXWindow *win) const {
  for(const FXEventLoop* eventloop=this; eventloop; eventloop=eventloop->upper){
    if(eventloop->modalType && eventloop->modalWindow==win) return true;
    }
  return false;
  }


// Break out of modal loop matching window
FXbool FXEventLoop::stopModal(FXWindow* win,FXint value){
  if(isModal(win)){
    for(FXEventLoop* eventloop=this; eventloop; eventloop=eventloop->upper){
      eventloop->code=0;
      eventloop->exit=true;
      if(eventloop->modalType && eventloop->modalWindow==win){
        eventloop->code=value;
        return true;
        }
      }
    }
  return false;
  }


// Break out of innermost modal loop, and all deeper non-modal ones
FXbool FXEventLoop::stopModal(FXint value){
  if(isModal()){
    for(FXEventLoop* eventloop=this; eventloop; eventloop=eventloop->upper){
      eventloop->code=0;
      eventloop->exit=true;
      if(eventloop->modalType){
        eventloop->code=value;
        return true;
        }
      }
    }
  return false;
  }


// Break out of all event loops
FXbool FXEventLoop::stop(FXint value){
  FXEventLoop* eventloop=this;
  while(eventloop->upper){
    eventloop->code=0;
    eventloop->exit=true;
    eventloop=eventloop->upper;
    }
  eventloop->code=value;
  return true;
  }


// Exit modal loop
FXEventLoop::~FXEventLoop(){
  *invocation=upper;
  }

}
