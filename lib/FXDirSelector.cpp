/********************************************************************************
*                                                                               *
*              D i r e c t o r y   S e l e c t i o n   W i d g e t              *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "fxkeys.h"
#include "FXMutex.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXElement.h"
#include "FXMetaClass.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSystem.h"
#include "FXPath.h"
#include "FXIO.h"
#include "FXStat.h"
#include "FXFile.h"
#include "FXDir.h"
#include "FXObjectList.h"
#include "FXStringDictionary.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXRecentFiles.h"
#include "FXFont.h"
#include "FXEvent.h"
#include "FXWindow.h"
#include "FXApp.h"
#include "FXGIFIcon.h"
#include "FXBMPIcon.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXMenuButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenuPane.h"
#include "FXScrollBar.h"
#include "FXTextField.h"
#include "FXScrollArea.h"
#include "FXTreeList.h"
#include "FXTreeListBox.h"
#include "FXVerticalFrame.h"
#include "FXHorizontalFrame.h"
#include "FXDirList.h"
#include "FXList.h"
#include "FXListBox.h"
#include "FXDirSelector.h"
#include "FXMenuCaption.h"
#include "FXMenuCommand.h"
#include "FXMenuCascade.h"
#include "FXMenuRadio.h"
#include "FXMenuCheck.h"
#include "FXMenuSeparator.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"
#include "FXInputDialog.h"
#include "FXSeparator.h"
#include "FXMessageBox.h"
#include "icons.h"


/*
  Notes:
  - Need a button to quickly hop to home directory.
  - Need a button to hop to current working directory.
  - Keep list of recently visited places.
  - Need button to hide/show hidden directories.
*/

#define DIRSTYLEMASK (TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|TREELIST_ROOT_BOXES)



using namespace FX;

/*******************************************************************************/

namespace FX {

// Map
FXDEFMAP(FXDirSelector) FXDirSelectorMap[]={
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_DIRNAME,FXDirSelector::onCmdName),
  FXMAPFUNC(SEL_OPENED,FXDirSelector::ID_DIRLIST,FXDirSelector::onCmdOpened),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,FXDirSelector::ID_DIRLIST,FXDirSelector::onPopupMenu),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_HOME,FXDirSelector::onCmdHome),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_WORK,FXDirSelector::onCmdWork),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_DIRECTORY_UP,FXDirSelector::onCmdDirectoryUp),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_VISIT,FXDirSelector::onCmdVisit),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_BOOKMARK,FXDirSelector::onCmdBookmark),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_UNBOOKMARK,FXDirSelector::onCmdUnBookmark),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_NEW,FXDirSelector::onCmdNew),
  FXMAPFUNC(SEL_UPDATE,FXDirSelector::ID_NEW,FXDirSelector::onUpdNew),
  FXMAPFUNC(SEL_UPDATE,FXDirSelector::ID_REMOVE,FXDirSelector::onUpdSelected),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_REMOVE,FXDirSelector::onCmdRemove),
  FXMAPFUNC(SEL_UPDATE,FXDirSelector::ID_MOVE,FXDirSelector::onUpdSelected),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_MOVE,FXDirSelector::onCmdMove),
  FXMAPFUNC(SEL_UPDATE,FXDirSelector::ID_COPY,FXDirSelector::onUpdSelected),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_COPY,FXDirSelector::onCmdCopy),
  FXMAPFUNC(SEL_UPDATE,FXDirSelector::ID_LINK,FXDirSelector::onUpdSelected),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_LINK,FXDirSelector::onCmdLink),
  };


// Implementation
FXIMPLEMENT(FXDirSelector,FXPacker,FXDirSelectorMap,ARRAYNUMBER(FXDirSelectorMap))


// Make directory selector widget
FXDirSelector::FXDirSelector(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXPacker(p,opts,x,y,w,h,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,8,8),mrufiles(p->getApp(),"Visited Directories"){
  FXString currentdirectory=FXSystem::getCurrentDirectory();
  FXAccelTable *table=getShell()->getAccelTable();
  target=tgt;
  message=sel;
  FXHorizontalFrame *buttons=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,0,0,0,0);
  accept=new FXButton(buttons,tr("&OK"),nullptr,nullptr,0,BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0,20,20);
  cancel=new FXButton(buttons,tr("&Cancel"),nullptr,nullptr,0,BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0,20,20);
  FXHorizontalFrame *field=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X,0,0,0,0,0,0,0,0);
  new FXLabel(field,tr("&Directory:"),nullptr,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
  dirname=new FXTextField(field,25,this,ID_DIRNAME,LAYOUT_FILL_X|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK);
  FXVerticalFrame *frame=new FXVerticalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(frame,tr("&Name"),nullptr,JUSTIFY_LEFT|JUSTIFY_CENTER_Y|LAYOUT_TOP|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X);
  dirbox=new FXDirList(frame,this,ID_DIRLIST,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|TREELIST_BROWSESELECT);
  updiricon=new FXGIFIcon(getApp(),dirup_gif);
  homeicon=new FXGIFIcon(getApp(),gotohome);
  workicon=new FXGIFIcon(getApp(),gotowork);
  bookmarkicon=new FXGIFIcon(getApp(),bookmrk_gif);
  bookaddicon=new FXGIFIcon(getApp(),bookset_gif);
  bookdelicon=new FXGIFIcon(getApp(),bookdel_gif);
  bookclricon=new FXGIFIcon(getApp(),bookclr_gif);
  sortingicon=new FXBMPIcon(getApp(),sorting,0,IMAGE_ALPHAGUESS);
  newicon=new FXGIFIcon(getApp(),foldernew);
  deleteicon=new FXGIFIcon(getApp(),filedelete);
  moveicon=new FXGIFIcon(getApp(),filemove);
  copyicon=new FXGIFIcon(getApp(),filecopy);
  linkicon=new FXGIFIcon(getApp(),filelink);
  mrufiles.setTarget(this);
  mrufiles.setSelector(ID_VISIT);
  if(table){
    table->addAccel(MKUINT(KEY_BackSpace,0),this,FXSEL(SEL_COMMAND,FXDirSelector::ID_DIRECTORY_UP));
    table->addAccel(MKUINT(KEY_Delete,0),this,FXSEL(SEL_COMMAND,FXDirSelector::ID_REMOVE));
    table->addAccel(MKUINT(KEY_h,CONTROLMASK),this,FXSEL(SEL_COMMAND,FXDirSelector::ID_HOME));
    table->addAccel(MKUINT(KEY_w,CONTROLMASK),this,FXSEL(SEL_COMMAND,FXDirSelector::ID_WORK));
    table->addAccel(MKUINT(KEY_n,CONTROLMASK),this,FXSEL(SEL_COMMAND,FXDirSelector::ID_NEW));
    }
  dirbox->setDirectory(currentdirectory);
  dirname->setText(currentdirectory);
  dirbox->setFocus();
  }


// Set directory
void FXDirSelector::setDirectory(const FXString& path){
  dirname->setText(path);
  dirbox->setDirectory(path);
  }


// Return directory
FXString FXDirSelector::getDirectory() const {
  return dirname->getText();
  }


// Change wildcard matching pattern
void FXDirSelector::setPattern(const FXString& ptrn){
  dirbox->setPattern(ptrn);
  }


// Return wildcard pattern
FXString FXDirSelector::getPattern() const {
  return dirbox->getPattern();
  }


// Return true if showing files as well as directories
FXbool FXDirSelector::showFiles() const {
  return dirbox->showFiles();
  }


// Show or hide normal files
void FXDirSelector::showFiles(FXbool showing){
  dirbox->showFiles(showing);
  }


// Return true if showing hidden files
FXbool FXDirSelector::showHiddenFiles() const {
  return dirbox->showHiddenFiles();
  }


// Show or hide hidden files
void FXDirSelector::showHiddenFiles(FXbool showing){
  dirbox->showHiddenFiles(showing);
  }


// Change wildcard matching mode
void FXDirSelector::setMatchMode(FXuint mode){
  dirbox->setMatchMode(mode);
  }


// Return wildcard matching mode
FXuint FXDirSelector::getMatchMode() const {
  return dirbox->getMatchMode();
  }


// Change directory list style
void FXDirSelector::setDirBoxStyle(FXuint style){
  dirbox->setListStyle((dirbox->getListStyle()&~DIRSTYLEMASK) | (style&DIRSTYLEMASK));
  }



// Return directory list style
FXuint FXDirSelector::getDirBoxStyle() const {
  return dirbox->getListStyle()&DIRSTYLEMASK;
  }


// Typed in new directory name, open path in the tree
long FXDirSelector::onCmdName(FXObject*,FXSelector,void*){
  dirbox->setDirectory(dirname->getText());
  return 1;
  }


// Opened an item, making it the current one
long FXDirSelector::onCmdOpened(FXObject*,FXSelector,void* ptr){
  const FXTreeItem* item=(const FXTreeItem*)ptr;
  dirname->setText(dirbox->getItemPathname(item));
  return 1;
  }


// Back to home directory
long FXDirSelector::onCmdHome(FXObject*,FXSelector,void*){
  setDirectory(FXSystem::getHomeDirectory());
  return 1;
  }


// Back to current working directory
long FXDirSelector::onCmdWork(FXObject*,FXSelector,void*){
  setDirectory(FXSystem::getCurrentDirectory());
  return 1;
  }


// User clicked up directory button
long FXDirSelector::onCmdDirectoryUp(FXObject*,FXSelector,void*){
  setDirectory(FXPath::upLevel(getDirectory()));
  return 1;
  }


// Move to recent directory
long FXDirSelector::onCmdVisit(FXObject*,FXSelector,void* ptr){
  FXString path((const FXchar*)ptr);
  if(FXStat::exists(path)){
    setDirectory(path);
    return 1;
    }
  mrufiles.removeFile(path);
  getApp()->beep();
  return 1;
  }


// Bookmark this directory
long FXDirSelector::onCmdBookmark(FXObject*,FXSelector,void*){
  mrufiles.appendFile(dirbox->getDirectory());
  return 1;
  }


// Unbookmark this directory
long FXDirSelector::onCmdUnBookmark(FXObject*,FXSelector,void*){
  mrufiles.removeFile(dirbox->getDirectory());
  return 1;
  }


// Create new directory
long FXDirSelector::onCmdNew(FXObject*,FXSelector,void*){
  FXBMPIcon newfoldericon(getApp(),newfolder,0,IMAGE_ALPHAGUESS);
  FXString dir(getDirectory());
  FXString name(tr("folder"));
  if(FXInputDialog::getString(name,this,tr("Create New Directory"),"Create new directory in: "+dir,&newfoldericon)){
    FXString folder=FXPath::absolute(dir,name);
    if(FXStat::exists(folder)){
      FXMessageBox::error(this,MBOX_OK,tr("Already Exists"),"File or directory %s already exists.\n",folder.text());
      return 1;
      }
    if(!FXDir::create(folder)){
      FXMessageBox::error(this,MBOX_OK,tr("Cannot Create"),"Cannot create directory %s.\n",folder.text());
      return 1;
      }
    setDirectory(folder);
    }
  return 1;
  }


// Update create new directory
long FXDirSelector::onUpdNew(FXObject* sender,FXSelector,void*){
  sender->handle(this,FXStat::isAccessible(getDirectory(),FXIO::Writing)?FXSEL(SEL_COMMAND,ID_ENABLE):FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Copy file or directory
long FXDirSelector::onCmdCopy(FXObject*,FXSelector,void*){
  FXString oldname=dirbox->getCurrentFile();
  FXString newname=FXPath::directory(oldname)+PATHSEPSTRING "CopyOf"+FXPath::name(oldname);
  FXInputDialog inputdialog(this,tr("Copy File"),"Copy file from location:\n\n"+oldname+"\n\nto location:",nullptr,INPUTDIALOG_STRING,0,0,0,0);
  inputdialog.setText(newname);
  inputdialog.setNumColumns(60);
  if(inputdialog.execute()){
    newname=inputdialog.getText();
    if(!FXFile::copyFiles(oldname,newname,false)){
      FXMessageBox::error(this,MBOX_OK,tr("Error Copying File"),"Unable to copy file:\n\n%s  to:  %s.",oldname.text(),newname.text());
      }
    }
  return 1;
  }


// Move file or directory
long FXDirSelector::onCmdMove(FXObject*,FXSelector,void*){
  FXString oldname=dirbox->getCurrentFile();
  FXString newname=oldname;
  FXInputDialog inputdialog(this,tr("Move File"),"Move file from location:\n\n"+oldname+"\n\nto location:",nullptr,INPUTDIALOG_STRING,0,0,0,0);
  inputdialog.setText(newname);
  inputdialog.setNumColumns(60);
  if(inputdialog.execute()){
    newname=inputdialog.getText();
    if(!FXFile::moveFiles(oldname,newname,false)){
      FXMessageBox::error(this,MBOX_OK,tr("Error Moving File"),"Unable to move file:\n\n%s  to:  %s.",oldname.text(),newname.text());
      }
    }
  return 1;
  }


// Link file or directory
long FXDirSelector::onCmdLink(FXObject*,FXSelector,void*){
  FXString oldname=dirbox->getCurrentFile();
  FXString newname=FXPath::directory(oldname)+PATHSEPSTRING "LinkTo"+FXPath::name(oldname);
  FXInputDialog inputdialog(this,tr("Link File"),"Link file from location:\n\n"+oldname+"\n\nto location:",nullptr,INPUTDIALOG_STRING,0,0,0,0);
  inputdialog.setText(newname);
  inputdialog.setNumColumns(60);
  if(inputdialog.execute()){
    newname=inputdialog.getText();
    if(!FXFile::symlink(oldname,newname)){
      FXMessageBox::error(this,MBOX_YES_NO,tr("Error Linking File"),"Unable to link file:\n\n%s  to:  %s.",oldname.text(),newname.text());
      }
    }
  return 1;
  }


// Remove file or directory
long FXDirSelector::onCmdRemove(FXObject*,FXSelector,void*){
  FXString fullname=dirbox->getCurrentFile();
  if(MBOX_CLICKED_YES==FXMessageBox::warning(this,MBOX_YES_NO,tr("Deleting file"),"Are you sure you want to delete the file:\n\n%s",fullname.text())){
    if(!FXFile::removeFiles(fullname,true)){
      FXMessageBox::error(this,MBOX_YES_NO,tr("Error Deleting File"),"Unable to delete file:\n\n%s.",fullname.text());
      }
    }
  return 1;
  }


// Sensitize when files are selected
long FXDirSelector::onUpdSelected(FXObject* sender,FXSelector,void*){
  sender->handle(this,dirbox->getCurrentItem()?FXSEL(SEL_COMMAND,ID_ENABLE):FXSEL(SEL_COMMAND,ID_DISABLE),nullptr);
  return 1;
  }


// Popup menu for item in file list
long FXDirSelector::onPopupMenu(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  if(event->moved) return 1;

  FXMenuPane filemenu(this);
  new FXMenuCommand(&filemenu,tr("Up one level"),updiricon,this,ID_DIRECTORY_UP);
  new FXMenuCommand(&filemenu,tr("Home directory"),homeicon,this,ID_HOME);
  new FXMenuCommand(&filemenu,tr("Work directory"),workicon,this,ID_WORK);
  new FXMenuSeparator(&filemenu);

  FXMenuPane sortmenu(this);
  new FXMenuCascade(&filemenu,tr("Sorting"),sortingicon,&sortmenu);
  new FXMenuCheck(&sortmenu,tr("Reverse"),dirbox,FXDirList::ID_SORT_REVERSE);
  new FXMenuCheck(&sortmenu,tr("Ignore case"),dirbox,FXDirList::ID_SORT_CASE);
  new FXMenuCheck(&sortmenu,tr("Hidden files"),dirbox,FXDirList::ID_TOGGLE_HIDDEN);

  FXMenuPane bookmenu(this);
  new FXMenuCascade(&filemenu,tr("Bookmarks"),bookmarkicon,&bookmenu);
  new FXMenuCommand(&bookmenu,tr("Set bookmark"),bookaddicon,this,ID_BOOKMARK);
  new FXMenuCommand(&bookmenu,tr("&Unset bookmark"),bookdelicon,this,ID_UNBOOKMARK);
  new FXMenuCommand(&bookmenu,tr("Clear bookmarks"),bookclricon,&mrufiles,FXRecentFiles::ID_CLEAR);
  FXMenuSeparator* sep1=new FXMenuSeparator(&bookmenu);
  sep1->setTarget(&mrufiles);
  sep1->setSelector(FXRecentFiles::ID_ANYFILES);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_1);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_2);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_3);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_4);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_5);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_6);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_7);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_8);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_9);
  new FXMenuCommand(&bookmenu,FXString::null,nullptr,&mrufiles,FXRecentFiles::ID_FILE_10);

  new FXMenuSeparator(&filemenu);
  new FXMenuCommand(&filemenu,tr("New directory..."),newicon,this,ID_NEW);
  new FXMenuCommand(&filemenu,tr("Copy..."),copyicon,this,ID_COPY);
  new FXMenuCommand(&filemenu,tr("Move..."),moveicon,this,ID_MOVE);
  new FXMenuCommand(&filemenu,tr("Link..."),linkicon,this,ID_LINK);
  new FXMenuCommand(&filemenu,tr("Delete..."),deleteicon,this,ID_REMOVE);

  filemenu.create();
  filemenu.popup(nullptr,event->root_x,event->root_y);
  getApp()->runModalWhileShown(&filemenu);
  return 1;
  }


// Change file associations
void FXDirSelector::setAssociations(FXFileAssociations* assoc,FXbool owned){
  dirbox->setAssociations(assoc,owned);
  }


// Return file associations
FXFileAssociations* FXDirSelector::getAssociations() const {
  return dirbox->getAssociations();
  }


// Save data
void FXDirSelector::save(FXStream& store) const {
  FXPacker::save(store);
  store << dirbox;
  store << dirname;
  store << accept;
  store << cancel;
  store << updiricon;
  store << homeicon;
  store << workicon;
  store << bookmarkicon;
  store << bookaddicon;
  store << bookdelicon;
  store << bookclricon;
  store << sortingicon;
  store << newicon;
  store << deleteicon;
  store << moveicon;
  store << copyicon;
  store << linkicon;
  }


// Load data
void FXDirSelector::load(FXStream& store){
  FXPacker::load(store);
  store >> dirbox;
  store >> dirname;
  store >> accept;
  store >> cancel;
  store >> updiricon;
  store >> homeicon;
  store >> workicon;
  store >> bookmarkicon;
  store >> bookaddicon;
  store >> bookdelicon;
  store >> bookclricon;
  store >> sortingicon;
  store >> newicon;
  store >> deleteicon;
  store >> moveicon;
  store >> copyicon;
  store >> linkicon;
  }


// Clean up
FXDirSelector::~FXDirSelector(){
  FXAccelTable *table=getShell()->getAccelTable();
  if(table){
    table->removeAccel(MKUINT(KEY_BackSpace,0));
    table->removeAccel(MKUINT(KEY_h,CONTROLMASK));
    table->removeAccel(MKUINT(KEY_w,CONTROLMASK));
    }
  delete updiricon;
  delete homeicon;
  delete workicon;
  delete bookmarkicon;
  delete bookaddicon;
  delete bookdelicon;
  delete bookclricon;
  delete sortingicon;
  delete newicon;
  delete deleteicon;
  delete moveicon;
  delete copyicon;
  delete linkicon;
  dirbox=(FXDirList*)-1L;
  dirname=(FXTextField*)-1L;
  accept=(FXButton*)-1L;
  cancel=(FXButton*)-1L;
  updiricon=(FXIcon*)-1L;
  homeicon=(FXIcon*)-1L;
  workicon=(FXIcon*)-1L;
  bookmarkicon=(FXIcon*)-1L;
  bookaddicon=(FXIcon*)-1L;
  bookdelicon=(FXIcon*)-1L;
  bookclricon=(FXIcon*)-1L;
  sortingicon=(FXIcon*)-1L;
  newicon=(FXIcon*)-1L;
  deleteicon=(FXIcon*)-1L;
  moveicon=(FXIcon*)-1L;
  copyicon=(FXIcon*)-1L;
  linkicon=(FXIcon*)-1L;
  }

}
