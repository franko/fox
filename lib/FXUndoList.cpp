/********************************************************************************
*                                                                               *
*                  U n d o / R e d o - a b l e   C o m m a n d                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2022 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "FXPtrList.h"
#include "FXArray.h"
#include "FXMarkedPtr.h"
#include "FXString.h"
#include "FXRectangle.h"
#include "FXEvent.h"
#include "FXWindow.h"
#include "FXEvent.h"
#include "FXWindow.h"
#include "FXUndoList.h"


/*
  Notes:

  - Manage general-purpose undo/redo capabilities, with the following
    special features:

    1) Unlimited undo/redo capability, limited only by RAM.

    2) Full retracing of history, even if, after undoing a bunch of
       undo commands, you decide to take a different path forward.

    3) Depending on allowable memory footprint, you can trim the
       number of undo records periodically, to keep them in check:

       a) Trim to a maximum number of records.

       b) Trim to a maximum memory-usage allowance.

       c) Trim non-linear history.

    4) A special mark-point may be set. You can quickly undo or redo
       commands to get to this mark point.

    5) Command groups are executed as a whole, allowing you to break
       up complicated modifications into smaller sub-units (FXCommands)
       which are undone/redone in a batch.

    6) Command groups may be nested to build ever larger commands as
       collection of sub-commands.

  - Commands must include all the information to both undo as well as
    redo the modification to a program's state.

  - Each command MUST reimplement both undo() as well as redo().

  - Each command SHOULD implement size() API, so trimming may be performed
    to keep space utilization within bounds.

  - Each command may implement undoName() and redoName(), if these are
    to be shown in your user interface in some way.

  - To undo large number of small changes (like typing a word of text),
    undo commands can optionally be merged into existing undo commands.
    For example, after typing the word "two words", an undo could roll
    back to "two "  instead of "two word".  This technique could reduce
    the number of undo records by unrolling only word-level modifications.

  - Undo record merging requires the developer to implement the canMerge()
    and mergeWith() APIs in subclasses of FXCommand.
    The parameter "merge" must be set to true in the add() API if a newly
    added undo-record is to be considered for a merge.

  - When setting a mark, subsequent undos and redos will increment or
    decrement the mark counter.  You can then revert to the marked state
    at any time, and restore to that state.  This is typically used
    to flag the state as "clean" when saving it to disk, for instance.

    Note that trimming undos past the marked state will prevent a revert
    to the marked state.

  - The busy() API will tell the user of FXUndoList that its in the middle
    of performing undos or redos.
    This is used to prevent generating new undo records while undoing or
    redoing operations are in progres..

  - FXCommand is now derived from FXObject.  The rationale behind this is
    that we may let FXCommand send messages to manipulate some target
    object, in effect driving it via messages.

  - Added "trim to marked" capability.  This is a sensible option as the
    marked point is often the state of where the file was last saved;
    history past that point is often not necessary and could be released
    if space is exceeded.

  - FIXME want trimAltHistory() to trim non-linear history only.  Keeping
    track of alternate history may not be as important as linear history;
    when space is exceeded one might want to start deleting alternate
    history before deleting linear history.
*/

using namespace FX;

/*******************************************************************************/

namespace FX {


// Object implementation
FXIMPLEMENT_ABSTRACT(FXCommand,FXObject,nullptr,0)


// Default implementation of undo name is just "Undo"
FXString FXCommand::undoName() const { return "Undo"; }


// Default implementation of redo name is just "Redo"
FXString FXCommand::redoName() const { return "Redo"; }


// Allow merging is false by default
FXbool FXCommand::canMerge() const { return false; }


// Don't merge by default
FXuint FXCommand::mergeWith(FXCommand*){ return 0; }


// Default returns size of undo record itself
FXuval FXCommand::size() const { return sizeof(FXCommand); }

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXCommandGroup,FXCommand,nullptr,0)


// Undoing a command group undoes each sub command
void FXCommandGroup::undo(){
  for(FXival i=command.no()-1; i>=0; --i){
    command[i]->undo();
    }
  }


// Undoing a command group undoes each sub command
void FXCommandGroup::redo(){
  for(FXival i=0; i<command.no(); ++i){
    command[i]->redo();
    }
  }


// Return the size of the information in the undo command group.
FXuval FXCommandGroup::size() const {
  FXuval result=sizeof(FXCommandGroup);
  for(FXival i=0; i<command.no(); ++i){
    result+=command[i]->size();
    }
  return result;
  }


// Clear list
void FXCommandGroup::clear(){
  for(FXival i=0; i<command.no(); ++i){
    command[i]->unref();
    }
  command.clear();
  delete group;
  }



// Destrying the command group destroys the subcommands
FXCommandGroup::~FXCommandGroup(){
  clear();
  }

/*******************************************************************************/

// Map
FXDEFMAP(FXUndoList) FXUndoListMap[]={
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_CLEAR,      FXUndoList::onCmdClear),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_CLEAR,      FXUndoList::onUpdClear),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REVERT,     FXUndoList::onCmdRevert),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REVERT,     FXUndoList::onUpdRevert),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_UNDO,       FXUndoList::onCmdUndo),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_UNDO,       FXUndoList::onUpdUndo),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REDO,       FXUndoList::onCmdRedo),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REDO,       FXUndoList::onUpdRedo),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_UNDO_ALL,   FXUndoList::onCmdUndoAll),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_UNDO_ALL,   FXUndoList::onUpdUndo),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REDO_ALL,   FXUndoList::onCmdRedoAll),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REDO_ALL,   FXUndoList::onUpdRedo),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_UNDO_COUNT, FXUndoList::onUpdUndoCount),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REDO_COUNT, FXUndoList::onUpdRedoCount),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_ALT_HISTORY,FXUndoList::onCmdAltHistory),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_ALT_HISTORY,FXUndoList::onUpdAltHistory),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_DUMP_STATS, FXUndoList::onCmdDumpStats),
  };


// Object implementation
FXIMPLEMENT(FXUndoList,FXCommandGroup,FXUndoListMap,ARRAYNUMBER(FXUndoListMap))



// Make new empty undo list
FXUndoList::FXUndoList():space(0),undocount(0),redocount(0),marker(0),markset(false),alternate(true),working(false){
  }


// Mark current state
void FXUndoList::mark(){
  markset=true;
  marker=0;
  }


// Unmark undo list
void FXUndoList::unmark(){
  markset=false;
  marker=0;
  }


// Check if marked
FXbool FXUndoList::marked() const {
  return markset && !marker && !group;
  }


// Return name of the first undo command available, if any
FXString FXUndoList::undoName() const {
  if(0<undocount){
    return command[undocount-1].flag() ? command[undocount-1]->undoName() : command[undocount-1]->redoName();
    }
  return FXString::null;
  }


// Return name of the first redo command available, if any
FXString FXUndoList::redoName() const {
  if(0<redocount){
    return command[undocount].flag() ? command[undocount]->redoName() : command[undocount]->undoName();
    }
  return FXString::null;
  }


// Size of undo information
FXuval FXUndoList::size() const {
  return space;
  }


// Can we undo more commands
FXbool FXUndoList::canUndo() const {
  return undocount>0;
  }


// Can we redo more commands
FXbool FXUndoList::canRedo() const {
  return redocount>0;
  }


// Can revert to marked
FXbool FXUndoList::canRevert() const {
  return markset && (marker!=0);
  }


// Return top undo command
FXCommand* FXUndoList::current() const {
  return (0<undocount) ? command[undocount-1] : nullptr;
  }


// Undo last command
void FXUndoList::undo(){
  if(group){ fxerror("FXUndoList::undo: cannot call undo inside begin-end block.\n"); }
  if(0<undocount){
    working=true;

    // Pop off undolist BEFORE undo
    undocount--;

    // Undo of forward means undo
    if(command[undocount].flag()){
      command[undocount]->undo();
      marker--;
      }
    else{
      command[undocount]->redo();
      marker++;
      }

    // Push to redolist AFTER undo
    redocount++;

    working=false;
    FXTRACE((100,"FXUndoList::undo: space=%lu undocount=%d redocount=%d marker=%d\n",space,undoCount(),redoCount(),marker));
    }
  }


// Redo next command
void FXUndoList::redo(){
  if(group){ fxerror("FXUndoList::redo: cannot call redo inside begin-end block.\n"); }
  if(0<redocount){
    working=true;

    // Pop off redolist BEFORE redo
    redocount--;

    // Redo of forward means redo
    if(command[undocount].flag()){
      command[undocount]->redo();
      marker++;
      }
    else{
      command[undocount]->undo();
      marker--;
      }

    // Push to undolist AFTER redo
    undocount++;

    working=false;
    FXTRACE((100,"FXUndoList::redo: space=%lu undocount=%d redocount=%d marker=%d\n",space,undoCount(),redoCount(),marker));
    }
  }


// Undo all commands
void FXUndoList::undoAll(){
  while(canUndo()) undo();
  }


// Redo all commands
void FXUndoList::redoAll(){
  while(canRedo()) redo();
  }


// Revert to marked
void FXUndoList::revert(){
  if(markset){
    while(marker>0) undo();
    while(marker<0) redo();
    }
  }


// We are taking an alternative path to the future.  After backing up from
// state S' to state S, we are now about to go to alternate state S''.
//
// Ordinarily, we would lose the capability to reach state S' after this
// operation; however, what if we wanted reach back to S' later on?
//
// The Great Undo Quandary is solved as follows:
//
// If, while in state S, there are any outstanding redo-commands on the
// redolist that would get us from state S to state S':
//
//  redolist: C1 C2 C3 C4
//
//  undolist: ...
//
// Instead of deleting commands C1...C4, as we would do before, we're now
// piling them back to the undolist, followed by their inverses, in reverse
// order, as shown below:
//
//  redolist: <empty>
//                              _  _  _  _
//  undolist: ...  C1 C2 C3 C4  C4 C3 C2 C1
//
// We are still in state S, the document doesn't get changed by this!
//
// However, we are now able to walk "back" from state S to state S',
// undoing the inverse of C1, inverse of C2, etc, until we get to S'.
// If we continue to go back, undoing C4, C3, etc., we will get to S
// again.  The entire sequence of commands thus has NO net effect on
// the document, if executed in its entirety!
//
// Assuming undo() is the opposite of redo(), and vice-versa, we can save
// VAST quantities of memory by sharing the original commands and simply
// keeping track of a flag to indicate if we're performing a command
// in reverse or not.
//
// The special class FXMarkedPtr is a "smart" pointer which keeps an extra
// flag around for this purpose.  It is no larger than a pointer, taking
// advantage of the fact that all allocations will return memory aligned
// to natural boundaries for the processor (typically 4- or 8-bytes, or
// more) to squirrel away an extra boolean into the pointer value.
//
// Thus, the "overhead" of this system is only a single pointer's worth,
// and each the command is recorded only once regardless how many times
// histroy folds back on itself!
// Reference counting the commands will allow us to decide when to finally
// delete them.
//
// Also note that "marker" does not change as an equal number of redo's and
// undo's are piled on....
//
FXbool FXUndoList::cut(){
  if(0<redocount){

    // Alternate history mode
    if(alternate){
      FXint total=undocount+redocount+redocount;

      // Make room if we're short
      if(command.no()<total && !command.no(total)) return false;

      undocount+=redocount;

      // Add reverse order, inverted
      for(FXival i=0; i<redocount; ++i){
        command[undocount+i]=command[undocount-1-i];
        command[undocount+i]->ref();
        command[undocount+i].flip();
        }

      // Update the books
      undocount+=redocount;
      redocount=0;
      FXTRACE((100,"FXUndoList::cut: space=%lu undocount=%d redocount=%d marker=%d\n",space,undoCount(),redoCount(),marker));
      return true;
      }

    // Linear history mode
    for(FXival i=0; i<redocount; ++i){
      if(command[undocount+i]->nrefs()==1){ space-=command[undocount+i]->size(); }
      command[undocount+i]->unref();
      }

    // Update the books
    redocount=0;
    FXTRACE((100,"FXUndoList::cut: space=%lu undocount=%d redocount=%d marker=%d\n",space,undoCount(),redoCount(),marker));
    }
  return true;
  }


// Add new command, executing if desired
FXbool FXUndoList::add(FXCommand* cmd,FXbool doit,FXbool merge){
  FXCommandGroup* grp=this;

  // Must pass a command
  if(!cmd){ fxerror("FXUndoList::add: NULL command argument.\n"); }

  // Adding undo while in the middle of doing something!
  if(working){ fxerror("FXUndoList::add: already working on undo or redo.\n"); }

  // Cut redo list
  if(cut()){

    working=true;

    // Hunt for end of group chain
    while(grp->group){ grp=grp->group; }

    // Execute command
    if(doit) cmd->redo();

    // Before allowing a merge to a prior undo command, we need to check:
    //
    //  1) Merging is to be performed,
    //  2) Current state is not marked (merging would not allow a revert)
    //  3) There actually is a prior command (duh!),
    //  4) The undo command is a forward one,
    //  5) There is no reference to the undo record's command in a prior undo,
    //  6) The command itself says its OK to merge.
    //
    // When the merge goes through, the merge may totally cancel the prior
    // command.  In that case, we might choose to drop both the new command
    // and the prior undo command.
    if(merge && !marked() && 0<grp->command.no()){

      // Previous command and flag
      FXCommand* prv=grp->command.tail();
      FXbool fwd=grp->command.tail().flag();

      // Merging could either adjust the previous command,
      // or even totally cancel it.  Either way, bookkeeping
      // must be adjusted if this happens at the top level.
      if(fwd && prv->nrefs()==1 && prv->canMerge()){
        FXuval oldsize=prv->size();             // Old size
        FXuint code=prv->mergeWith(cmd);        // Try merge
        FXuval newsize=prv->size();             // New size
        if(1<=code){
          if(2<=code){

            // Remove last
            grp->command.pop();

            // Remove reference
            prv->unref();

            newsize=0;

            // Update bookkeeping
            if(grp==this){
              undocount--;
              marker-=1;
              }
            }

          // Update bookkeeping
          if(grp==this){
            space+=newsize-oldsize;
            }

          // Delete command
          delete cmd;

          working=false;
          FXTRACE((100,"FXUndoList::add: space=%lu undocount=%d marker=%d\n",space,undoCount(),marker));
          return true;
          }
        }
      }

    // Create space if not fits
    if(undocount<command.no() || command.no(command.no()+1)){

      FXASSERT(undocount<command.no());

      // Append incoming command
      command[undocount]=cmd;

      // Going forward
      command[undocount].flag(true);

      // Add reference
      cmd->ref();

      // Update bookkeeping
      if(grp==this){
        space+=cmd->size();
        undocount++;
        marker++;
        }

      working=false;

      FXTRACE((100,"FXUndoList::add: space=%lu undocount=%d marker=%d\n",space,undoCount(),marker));
      return true;
      }
    working=false;
    }
  return false;
  }


// Begin a new undo command group
FXbool FXUndoList::begin(FXCommandGroup *cmd){
  FXCommandGroup* grp=this;

  // Must pass a command group
  if(!cmd){ fxerror("FXUndoList::begin: NULL command argument.\n"); }

  // Calling begin while in the middle of doing something!
  if(working){ fxerror("FXUndoList::begin: already working on undo or redo.\n"); }

  // Cut redo list
  if(cut()){

    // Hunt for end of group chain
    while(grp->group){ grp=grp->group; }

    // Add to end
    grp->group=cmd;

    return true;
    }
  return false;
  }


// End undo command group
FXbool FXUndoList::end(){
  FXCommandGroup* grp=this;
  FXCommandGroup* cmd;

  // Must have called begin
  if(!grp->group){ fxerror("FXUndoList::end: no matching call to begin.\n"); }

  // Calling end while in the middle of doing something!
  if(working){ fxerror("FXUndoList::end: already working on undo or redo.\n"); }

  // Hunt for one above end of group chain
  while(grp->group->group){ grp=grp->group; }

  // Unlink from group chain
  cmd=grp->group;
  grp->group=nullptr;

  // Add to group if non-empty
  if(!cmd->empty()){

    // Append new command to undo list
    grp->command.push(cmd);

    // Update bookkeeping
    if(grp==this){
      space+=cmd->size();
      undocount++;
      marker+=1;
      }
    return true;
    }

  // Delete bottom group
  delete cmd;

  // Not added
  return false;
  }


// Abort undo command group
FXbool FXUndoList::abort(){
  FXCommandGroup *grp=this;

  // Must be called after begin
  if(!grp->group){ fxerror("FXUndoList::abort: no matching call to begin.\n"); }

  // Calling abort while in the middle of doing something!
  if(working){ fxerror("FXUndoList::abort: already working on undo or redo.\n"); }

  // Hunt for one above end of group chain
  while(grp->group->group){ grp=grp->group; }

  // Delete bottom group
  delete grp->group;

  // New end of chain
  grp->group=nullptr;

  return true;
  }


// Clear list
void FXUndoList::clear(){
  FXTRACE((100,"FXUndoList::clear: space=%lu undocount=%d redocount=%d marker=%d\n",space,undoCount(),redoCount(),marker));
  FXCommandGroup::clear();
  space=0;
  undocount=0;
  redocount=0;
  marker=0;
  markset=false;
  working=false;
  }


// Trim undo list down to at most nc records
void FXUndoList::trimCount(FXint nc){
  FXTRACE((100,"FXUndoList::trimCount: was: space=%lu undocount=%d; marker=%d ",space,undocount,marker));
  if(nc<undocount){
    FXint i=0;
    while(i<undocount-nc){
      if(command[i]->nrefs()==1){ space-=command[i]->size(); }
      command[i]->unref();
      i++;
      }
    command.erase(0,i);
    undocount-=i;
    if(undocount<marker) markset=false;
    }
  FXTRACE((100,"now: space=%lu undocount=%d; marker=%d\n",space,undocount,marker));
  }


// Trim undo list down to at most size sz
void FXUndoList::trimSize(FXuval sz){
  FXTRACE((100,"FXUndoList::trimSize: was: space=%lu undocount=%d; marker=%d ",space,undocount,marker));
  if(sz<space){
    FXint i=0;
    while(i<undocount && sz<space){
      if(command[i]->nrefs()==1){ space-=command[i]->size(); }
      command[i]->unref();
      i++;
      }
    command.erase(0,i);
    undocount-=i;
    if(undocount<marker) markset=false;
    }
  FXTRACE((100,"now: space=%lu undocount=%d; marker=%d\n",space,undocount,marker));
  }


// Trim undo list down to (but not including) marked node.
void FXUndoList::trimMark(){
  FXTRACE((100,"FXUndoList::trimSize: was: space=%lu undocount=%d; marker=%d ",space,undocount,marker));
  if(markset && marker<undocount){
    FXint i=0;
    while(i<undocount-marker){
      if(command[i]->nrefs()==1){ space-=command[i]->size(); }
      command[i]->unref();
      i++;
      }
    command.erase(0,i);
    undocount-=i;
    FXASSERT(undocount==marker);
    }
  FXTRACE((100,"now: space=%lu undocount=%d; marker=%d\n",space,undocount,marker));
  }


// Dump statistics
void FXUndoList::dumpStats(){
  const FXchar onoff[2][6]={"false","true"};
  FXCommand *cmd;
  FXbool forward;
  fxmessage("FXUndoList stats:\n");
  fxmessage("  memory used       : %lu\n",space);
  fxmessage("  number of records : %lu\n",command.no());
  fxmessage("  undolist length   : %d\n",undocount);
  fxmessage("  redolist length   : %d\n",redocount);
  fxmessage("  marker            : %d (set: %s)\n",marker,onoff[markset]);
  fxmessage("  alternate history : %s\n",onoff[alternate]);
  fxmessage("  undolist : \n");
  for(FXival i=0; i<undocount; ++i){
    cmd=command[i];
    forward=command[i].flag();
    fxmessage("    %p: name: %30s  size: %5lu  refs: %2ld  dir: %c\n",cmd,cmd->getClassName(),cmd->size(),cmd->nrefs(),forward?'F':'B');
    }
  fxmessage("  redolist:%d:\n",redocount);
  for(FXival i=undocount; i<undocount+redocount; ++i){
    cmd=command[i];
    forward=command[i].flag();
    fxmessage("    %p: name: %30s  size: %5lu  refs: %2ld  dir: %c\n",cmd,cmd->getClassName(),cmd->size(),cmd->nrefs(),forward?'F':'B');
    }
  }


// Clear undo list
long FXUndoList::onCmdClear(FXObject*,FXSelector,void*){
  clear();
  return 1;
  }


// Update Clear undo list
long FXUndoList::onUpdClear(FXObject* sender,FXSelector,void*){
  sender->handle(this,(canUndo()||canRedo())?FXSEL(SEL_COMMAND,FXWindow::ID_ENABLE):FXSEL(SEL_COMMAND,FXWindow::ID_DISABLE),nullptr);
  return 1;
  }


// Revert to marked
long FXUndoList::onCmdRevert(FXObject*,FXSelector,void*){
  revert();
  return 1;
  }


// Update revert to marked
long FXUndoList::onUpdRevert(FXObject* sender,FXSelector,void*){
  sender->handle(this,canRevert()?FXSEL(SEL_COMMAND,FXWindow::ID_ENABLE):FXSEL(SEL_COMMAND,FXWindow::ID_DISABLE),nullptr);
  return 1;
  }


// Undo last command
long FXUndoList::onCmdUndo(FXObject*,FXSelector,void*){
  undo();
  return 1;
  }


// Undo all commands
long FXUndoList::onCmdUndoAll(FXObject*,FXSelector,void*){
  undoAll();
  return 1;
  }


// Update undo last command
long FXUndoList::onUpdUndo(FXObject* sender,FXSelector,void*){
  sender->handle(this,canUndo()?FXSEL(SEL_COMMAND,FXWindow::ID_ENABLE):FXSEL(SEL_COMMAND,FXWindow::ID_DISABLE),nullptr);
  return 1;
  }


// Redo last command
long FXUndoList::onCmdRedo(FXObject*,FXSelector,void*){
  redo();
  return 1;
  }


// Redo all commands
long FXUndoList::onCmdRedoAll(FXObject*,FXSelector,void*){
  redoAll();
  return 1;
  }


// Update redo last command
long FXUndoList::onUpdRedo(FXObject* sender,FXSelector,void*){
  sender->handle(this,canRedo()?FXSEL(SEL_COMMAND,FXWindow::ID_ENABLE):FXSEL(SEL_COMMAND,FXWindow::ID_DISABLE),nullptr);
  return 1;
  }


// Update undo count
long FXUndoList::onUpdUndoCount(FXObject* sender,FXSelector,void*){
  sender->handle(this,FXSEL(SEL_COMMAND,FXWindow::ID_SETINTVALUE),(void*)&undocount);
  return 1;
  }


// Update redo count
long FXUndoList::onUpdRedoCount(FXObject* sender,FXSelector,void*){
  sender->handle(this,FXSEL(SEL_COMMAND,FXWindow::ID_SETINTVALUE),(void*)&redocount);
  return 1;
  }


// Toggle alternate history mode
long FXUndoList::onCmdAltHistory(FXObject*,FXSelector,void*){
  alternate=!alternate;
  return 1;
  }


// Update alternate history mode
long FXUndoList::onUpdAltHistory(FXObject* sender,FXSelector,void*){
  sender->handle(this,alternate?FXSEL(SEL_COMMAND,FXWindow::ID_CHECK):FXSEL(SEL_COMMAND,FXWindow::ID_UNCHECK),nullptr);
  return 1;
  }


// Dump stats
long FXUndoList::onCmdDumpStats(FXObject*,FXSelector,void*){
  dumpStats();
  return 1;
  }


// Destroy now
FXUndoList::~FXUndoList(){
  clear();
  }

}
