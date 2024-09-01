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
#include "fx.h"
#include "Commands.h"
#include "Syntax.h"
#include "Modeline.h"
#include "TextWindow.h"
#include "Adie.h"


/*
  Notes:

  - New updated undo/redo command perform "smart" merging, i.e. individual
    character insertions or deletions are merged into bigger undo records,
    allowing a single ^Z to undo more than one charactor.
  - Inserts followed by deletes can be merged as well; this can eliminate
    the need to undo/redo over small typos that are corrected by backspacing
    and retyping.
  - We're still playing around with this feature; possible extensions being
    looked at are, for example, merging only characted insertions of the
    same type (e.g. digits, letters, spaces and tabs).  This would be great
    because this way a ^Z undoes only one lexical entity, instead of potentially
    removing an entire line.
  - For now, we limit the undo-merging to single character modifications as
    these are occurring during text enty.  The idea is to not undo/redo over
    trivial modifications.
  - When manipulating text widget, we generate callbacks even when changing
    the text via undo or redo.
  - During the execution of an undo or redo, FXUndoList is marked as busy;
    thus, the busy state may be checked to determine if one is in the middle
    of an undo or redo.
  - Keep undo-text around basically forever; but there are limits: when undo-
    list gets trimmed, old undo-commands may be deleted.
    We have two ways to trim the undo-buffer: by total undo information size or
    by the number of undo records.
    Either way, if the undo buffer ends up being trimmed, only the older undo
    records that are in excess of the chosen trim-limits are deleted; keeping
    the most recent undo records.
*/


/*******************************************************************************/

FXIMPLEMENT_ABSTRACT(FXTextCommand,FXCommand,nullptr,0)


// Initialize undo/redo command
// Contiguous single-letter changes to the buffer may be mergeable
FXTextCommand::FXTextCommand(FXText* txt,FXint p,FXbool mg):text(txt),pos(p),merge(mg){
  }


// Return size of record plus any data kept here
FXuval FXTextCommand::size() const {
  return sizeof(FXTextCommand);
  }


// We can merge
FXbool FXTextCommand::canMerge() const {
  return merge;
  }


/*******************************************************************************/

FXIMPLEMENT_ABSTRACT(FXTextInsert,FXTextCommand,nullptr,0)


// Insert command
FXTextInsert::FXTextInsert(FXText* txt,FXint p,FXint ni,const FXchar* ins):FXTextCommand(txt,p,(ni==1)),inserted(ins,ni){
  }


// Return size of record plus any data kept here
FXuval FXTextInsert::size() const {
  return sizeof(FXTextInsert)+inserted.length();
  }


// Return undo name for display in gui
FXString FXTextInsert::undoName() const {
  return "Undo insert";
  }


// Return redo name for display in gui
FXString FXTextInsert::redoName() const {
  return "Redo insert";
  }


// Merge consecutive inserts, or inserts followed by a delete
// This, to some extent, avoids undo/redo-ing spelling errors
// that are corrected by backspacing and retyping.
FXuint FXTextInsert::mergeWith(FXCommand* command){
  const FXushort PROPERTIES=Ascii::Word|Ascii::Blank|Ascii::Punct|Ascii::Digit;
  if(text->isModified()){
    FXTextInsert* txtins=dynamic_cast<FXTextInsert*>(command);
    if(txtins && txtins->inserted.length()==1 && txtins->pos==pos+inserted.length()){
      if(((Ascii::charProperties(inserted.tail())^Ascii::charProperties(txtins->inserted.tail()))&PROPERTIES)==0){
        inserted.append(txtins->inserted);
        return 1;
        }
      }
    FXTextDelete* txtdel=dynamic_cast<FXTextDelete*>(command);
    if(txtdel && txtdel->deleted.length()==1 && txtdel->pos==pos+inserted.length()-1){
      inserted.erase(txtdel->pos-pos,1);
      if(inserted.length()==0) return 2;
      return 1;
      }
    }
  return 0;
  }


// Undo an insert removes the inserted text
void FXTextInsert::undo(){
  text->removeText(pos,inserted.length(),true);
  text->setCursorPos(pos);
  text->makePositionVisible(pos);
  }


// Redo an insert inserts the same old text again
void FXTextInsert::redo(){
  text->insertText(pos,inserted,true);
  text->setCursorPos(pos+inserted.length());
  text->makePositionVisible(pos+inserted.length());
  }


// Destroy
FXTextInsert::~FXTextInsert(){
  }

/*******************************************************************************/

FXIMPLEMENT_ABSTRACT(FXTextDelete,FXTextCommand,nullptr,0)


// Delete command
FXTextDelete::FXTextDelete(FXText* txt,FXint p,FXint nd,const FXchar* del):FXTextCommand(txt,p,(nd==1)),deleted(del,nd){
  }


// Return size of record plus any data kept here
FXuval FXTextDelete::size() const {
  return sizeof(FXTextDelete)+deleted.length();
  }


// Return undo name for display in gui
FXString FXTextDelete::undoName() const {
  return "Undo delete";
  }


// Return redo name for display in gui
FXString FXTextDelete::redoName() const {
  return "Redo delete";
  }


// Merge consecutive deletes
FXuint FXTextDelete::mergeWith(FXCommand* command){
  if(text->isModified()){
    FXTextDelete* txtdel=dynamic_cast<FXTextDelete*>(command);
    if(txtdel && txtdel->deleted.length()==1){
      if(txtdel->pos==pos){
        deleted.append(txtdel->deleted);
        return 1;
        }
      if(txtdel->pos+txtdel->deleted.length()==pos){
        deleted.prepend(txtdel->deleted);
        pos=txtdel->pos;
        return 1;
        }
      }
    }
  return 0;
  }


// Undo a delete reinserts the old text
void FXTextDelete::undo(){
  text->insertText(pos,deleted,true);
  text->setCursorPos(pos+deleted.length());
  text->makePositionVisible(pos+deleted.length());
  }


// Redo a delete removes it again
void FXTextDelete::redo(){
  text->removeText(pos,deleted.length(),true);
  text->setCursorPos(pos);
  text->makePositionVisible(pos);
  }

// Destroy
FXTextDelete::~FXTextDelete(){
  }

/*******************************************************************************/

FXIMPLEMENT_ABSTRACT(FXTextReplace,FXTextCommand,nullptr,0)


// Replace command
FXTextReplace::FXTextReplace(FXText* txt,FXint p,FXint nd,FXint ni,const FXchar* del,const FXchar* ins):FXTextCommand(txt,p,((nd|ni)==1)),deleted(del,nd),inserted(ins,ni){
  }


// Return size of record plus any data kept here
FXuval FXTextReplace::size() const {
  return sizeof(FXTextReplace)+deleted.length()+inserted.length();
  }


// Return undo name for display in gui
FXString FXTextReplace::undoName() const {
  return "Undo replace";
  }


// Return redo name for display in gui
FXString FXTextReplace::redoName() const {
  return "Redo replace";
  }


// Merge consecutive overstrike commands
FXuint FXTextReplace::mergeWith(FXCommand* command){
  if(text->isModified()){
    FXTextReplace* txtrep=dynamic_cast<FXTextReplace*>(command);
    if(txtrep && txtrep->deleted.length()==1 && txtrep->inserted.length()==1){
      if(txtrep->pos==pos+inserted.length()){
        deleted.append(txtrep->deleted);
        inserted.append(txtrep->inserted);
        return 1;
        }
      }
    }
  return 0;
  }


// Undo a replace reinserts the old text
void FXTextReplace::undo(){
  text->replaceText(pos,inserted.length(),deleted,true);
  text->setCursorPos(pos+deleted.length());
  text->makePositionVisible(pos+deleted.length());
  }


// Redo a replace reinserts the new text
void FXTextReplace::redo(){
  text->replaceText(pos,deleted.length(),inserted,true);
  text->setCursorPos(pos+inserted.length());
  text->makePositionVisible(pos+inserted.length());
  }


// Destroy
FXTextReplace::~FXTextReplace(){
  }

