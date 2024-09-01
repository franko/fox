/********************************************************************************
*                                                                               *
*                     U n d o a b l e   C o m m a n d s                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This program is free software: you can redistribute it and/or modify          *
* it under the terms of the GNU General Public License as published by          *
* the Free Software Foundation, either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This program is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU General Public License for more details.                                  *
*                                                                               *
* You should have received a copy of the GNU General Public License             *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
********************************************************************************/
#ifndef COMMANDS_H
#define COMMANDS_H



// Undo record for text fragment
class FXTextCommand : public FXCommand {
  FXDECLARE_ABSTRACT(FXTextCommand)
protected:
  FXText  *text;        // Text widget
  FXint    pos;         // Character position
  FXbool   merge;       // Allow merge
protected:
  FXTextCommand(FXText* txt,FXint p,FXbool mg);
public:
  virtual FXuval size() const;
  virtual FXbool canMerge() const;
  };


// Insert command
class FXTextInsert : public FXTextCommand {
  FXDECLARE_ABSTRACT(FXTextInsert)
  friend class FXTextDelete;
  friend class FXTextReplace;
protected:
  FXString inserted;
public:
  FXTextInsert(FXText* txt,FXint p,FXint ni,const FXchar* ins);
  virtual FXuval size() const;
  virtual FXString undoName() const;
  virtual FXString redoName() const;
  virtual FXuint mergeWith(FXCommand* command);
  virtual void undo();
  virtual void redo();
  virtual ~FXTextInsert();
  };


// Delete command
class FXTextDelete : public FXTextCommand {
  FXDECLARE_ABSTRACT(FXTextDelete)
  friend class FXTextInsert;
  friend class FXTextReplace;
protected:
  FXString deleted;
public:
  FXTextDelete(FXText* txt,FXint p,FXint nd,const FXchar* del);
  virtual FXuval size() const;
  virtual FXString undoName() const;
  virtual FXString redoName() const;
  virtual FXuint mergeWith(FXCommand* command);
  virtual void undo();
  virtual void redo();
  virtual ~FXTextDelete();
  };


// Replace command
class FXTextReplace : public FXTextCommand {
  FXDECLARE_ABSTRACT(FXTextReplace)
  friend class FXTextInsert;
  friend class FXTextDelete;
protected:
  FXString deleted;
  FXString inserted;
public:
  FXTextReplace(FXText* txt,FXint p,FXint nd,FXint ni,const FXchar* del,const FXchar* ins);
  virtual FXuval size() const;
  virtual FXString undoName() const;
  virtual FXString redoName() const;
  virtual FXuint mergeWith(FXCommand* command);
  virtual void undo();
  virtual void redo();
  virtual ~FXTextReplace();
  };

#endif

