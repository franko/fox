/********************************************************************************
*                                                                               *
*                         F O X   E v e n t   L o o p                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 2019,2022 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#ifndef FXEVENTLOOP_H
#define FXEVENTLOOP_H

namespace FX {

// Forward declarations
class FXWindow;
class FXEventLoop;


/// Recursive event loop
class FXAPI FXEventLoop {
private:
  FXEventLoop      **invocation;        // Pointer to variable holding pointer to current invocation
  FXEventLoop       *upper;             // Invocation above this one
  FXWindow          *modalWindow;       // Modal window (if any)
  FXuint             modalType;         // Modal type
  FXint              code;              // Return code
  FXbool             exit;              // Exit flag
public:
  enum{
    ModalForNone   = 0,
    ModalForWindow = 1,
    ModalForPopup  = 2
    };
private:
  FXEventLoop(const FXEventLoop&);
  FXEventLoop& operator=(const FXEventLoop&);
public:

  /// Initialize event loop
  FXEventLoop(FXEventLoop** inv,FXWindow* win=nullptr,FXuint mode=0);

  /// Test if in a modal invocation
  FXbool isModal() const;

  /// Test if in a modal invocation for window
  FXbool isModal(FXWindow *win) const;

  /// Break out of modal loop
  FXbool stopModal(FXint value);

  /// Break out of modal loop for window
  FXbool stopModal(FXWindow* win,FXint value);

  /// Break out of all event loops
  FXbool stop(FXint value);

  /// Return window of current modal event loop
  FXWindow* window() const { return modalWindow; }

  /// Return mode of this model event loop
  FXuint modality() const { return modalType; }

  /// Return result-code of this loop
  FXint result() const { return code; }

  /// Return done flag
  FXbool done() const { return exit; }

  /// Destroy event loop
 ~FXEventLoop();
  };

}

#endif
