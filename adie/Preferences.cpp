/********************************************************************************
*                                                                               *
*                        P r e f e r e n c e s   D i a l o g                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2021 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "icons.h"
#include "help.h"
#include "Preferences.h"
#include "Syntax.h"
#include "Modeline.h"
#include "TextWindow.h"
#include "Adie.h"
#include "icons.h"


/*******************************************************************************/


// Object implementation
FXIMPLEMENT(Preferences,FXDialogBox,NULL,0)



// Construct
Preferences::Preferences(TextWindow *own):FXDialogBox(own,"Adie Preferences",DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,0,0,0,0, 0,0,0,0, 4,4){

  // Set title
  setTitle(tr("Adie Preferences"));

  // Interior
  FXVerticalFrame *vertical=new FXVerticalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame *horizontal=new FXHorizontalFrame(vertical,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame *buttons=new FXVerticalFrame(horizontal,LAYOUT_LEFT|LAYOUT_FILL_Y|FRAME_SUNKEN|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT,0,0,0,0, 0,0,0,0, 0,0);
  FXSwitcher *switcher=new FXSwitcher(horizontal,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

  // Icons
  pal=new FXGIFIcon(getApp(),palette_gif);
  ind=new FXGIFIcon(getApp(),indent_gif);
  pat=new FXGIFIcon(getApp(),pattern_gif);
  sty=new FXGIFIcon(getApp(),styles_gif);
  mis=new FXGIFIcon(getApp(),miscellaneous_gif);

  ///////////////////////////  Editor settings pane  ////////////////////////////
  FXVerticalFrame* editorpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(editorpane,tr("Editor settings"),NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(editorpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXMatrix *matrix2=new FXMatrix(editorpane,10,MATRIX_BY_ROWS|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4, 2);

  new FXLabel(matrix2,tr("Word wrapping:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Auto indent:\tAutomatically indent new line same as previous line\tAutomatically indent new line same as previous line."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Fixed wrap margin:\tWrap words at fixed margin instead of width of window\tWrap words at fixed margin instead of width of window."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Strip carriage returns:\tStrip carriage returns when loading files\tStrip carriage returns when loading files."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Append carriage returns:\tAppend carriage returns when saving files\tAppend carriage returns when saving files."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Strip trailing spaces:\tStrip useless spaces from ends of lines when saving files\tStrip useless spaces from ends of lines when saving files."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Append newline at end of file:\tAppend a newline at the end of the file if needed\tEnsure file ends with a newline when saved."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Insert tab characters:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Brace matching:\tHighlight matching braces, parentheses, or brackets\tHighlight matching braces, parentheses, or brackets."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Modeline parsing support:\tParse adie, emacs, or vim modelines\tParse adie, emacs, or vim modelines to set language and other attributes."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X);

  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_TOGGLE_WRAP,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_AUTOINDENT,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_FIXED_WRAP,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_STRIP_CR,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_APPEND_CR,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_STRIP_SP,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_APPEND_NL,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_INSERTTABS,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_BRACEMATCH,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_MODELINE,LAYOUT_LEFT|LAYOUT_CENTER_Y,0,0,0,0, 0,0,0,0);

  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

  new FXLabel(matrix2,tr("Wrap margin:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Tab columns:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Brace match time (ms):"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Mouse wheel lines:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Line number space:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Save view of file:\tSave and restore file view\tPermanently remember view for each file."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Save bookmarks:\tSave and restore bookmarks\tPermanently remember bookmark positions for each file."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Warn if changed externally:\tWarn if another program changed the file\tWarn if another program changed the file being edited."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,tr("Brace match stays:\tMatching brace stays highlighted\tMatching brace stays highlighted until cursor moves."),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

  new FXTextField(matrix2,6,own,TextWindow::ID_WRAPCOLUMNS,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  new FXTextField(matrix2,6,own,TextWindow::ID_TABCOLUMNS,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  new FXTextField(matrix2,6,own,TextWindow::ID_BRACEMATCHTIME,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  FXSpinner* spinner=new FXSpinner(matrix2,3,own,TextWindow::ID_WHEELADJUST,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  spinner->setRange(1,100);
  FXSpinner* spinner1=new FXSpinner(matrix2,3,own,TextWindow::ID_TEXT_LINENUMS,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  spinner1->setRange(0,8);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_SAVEVIEWS,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_SAVEMARKS,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_WARNCHANGED,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,FXString::null,own,TextWindow::ID_BRACEMATCHSTAY,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

  //// Editor settings button
  new FXButton(buttons,tr("Editor\tEditor settings\tChange editor settings and other things."),ind,switcher,FXSwitcher::ID_OPEN_FIRST,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  /////////////////////////  Color settings pane  ///////////////////////////////
  FXVerticalFrame* colorspane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(colorspane,tr("Color settings"),NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(colorspane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXMatrix *matrix1=new FXMatrix(colorspane,8,MATRIX_BY_ROWS|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4, 4);

  new FXLabel(matrix1,tr("Background:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Text:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Sel. text background:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Sel. text:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Hilite text background:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Hilite text:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Act. text background:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Numbers background:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);

  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_BACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_FORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_SELBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_SELFORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_HILITEBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_HILITEFORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_ACTIVEBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_NUMBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);

  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

  new FXLabel(matrix1,tr("Files background:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Files:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Sel. files background:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Sel. files:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Lines:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Cursor:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Active background:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,tr("Numbers:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);

  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_DIR_BACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_DIR_FORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_DIR_SELBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_DIR_SELFORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_DIR_LINES,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_CURSOR,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXCheckButton(matrix1,FXString::null,own,TextWindow::ID_SHOWACTIVE,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 5,5,0,0);
  new FXColorWell(matrix1,FXRGB(0,0,0),own,TextWindow::ID_TEXT_NUMFORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);

  //// Color settings button
  new FXButton(buttons,tr("Colors\tChange Colors\tChange text colors."),pal,switcher,FXSwitcher::ID_OPEN_SECOND,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  ///////////////////////  File pattern settings pane  //////////////////////////
  FXVerticalFrame* filepatpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(filepatpane,tr("Filename patterns"),NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(filepatpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXVerticalFrame *sub3=new FXVerticalFrame(filepatpane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,10,0, 0,0);
  new FXLabel(sub3,tr("Filename patterns, one per line:"),NULL,JUSTIFY_LEFT);
  FXVerticalFrame* textwell=new FXVerticalFrame(sub3,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  filepattext=new FXText(textwell,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  //// File pattern settings button
  new FXButton(buttons,tr("Patterns\tFilename patterns\tChange wildcard patterns for filenames."),pat,switcher,FXSwitcher::ID_OPEN_THIRD,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  /////////////////////  Highlight style settings pane  /////////////////////////
  FXVerticalFrame* highlightpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(highlightpane,tr("Highlight styles"),NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(highlightpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXHorizontalFrame *sub5=new FXHorizontalFrame(highlightpane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,10,0);

  // Scroll window inside sunken frame
  FXHorizontalFrame *sub6=new FXHorizontalFrame(sub5,LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  FXScrollWindow *styles=new FXScrollWindow(sub6,HSCROLLING_OFF|LAYOUT_FILL_Y|LAYOUT_FILL_X);

  // Matrix of style entries
  stylemat=new FXMatrix(styles,11,MATRIX_BY_COLUMNS|LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 0,0);
  stylemat->setBackColor(getApp()->getBackColor());
  stylemat->enable();

  //// Highlight style settings button
  new FXButton(buttons,tr("Styles\tHighlight styles\tChange highlight styles for syntax coloring."),sty,switcher,FXSwitcher::ID_OPEN_FOURTH,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);

  ///////////////////////  File paths settings pane  //////////////////////////
  FXVerticalFrame* miscellaneouspane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(miscellaneouspane,tr("Miscellaneous settings"),NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(miscellaneouspane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXMatrix *matrix3=new FXMatrix(miscellaneouspane,2,MATRIX_BY_COLUMNS|LAYOUT_FILL_X,0,0,0,0, 4,4,4,4, 4, 4);

  new FXLabel(matrix3,tr("Search paths:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
  searchtext=new FXTextField(matrix3,10,own,TextWindow::ID_SEARCHPATHS,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN,0,0,0,0, 2,2,1,1);
  searchtext->setTipText(tr("List of directories separated by a '" PATHLISTSEPSTRING "' where include files are to be found.\nPaths are subjected to tilde and environment variable expansion."));
  searchtext->setHelpText(tr("Change file search path."));

  new FXLabel(matrix3,tr("Syntax path:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
  syntaxtext=new FXTextField(matrix3,10,getApp(),Adie::ID_SYNTAXPATHS,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN,0,0,0,0, 2,2,1,1);
  syntaxtext->setTipText(tr("List of directories separated by a '" PATHLISTSEPSTRING "' where syntax file is to be found."));
  syntaxtext->setHelpText(tr("Change syntax file search path."));

  new FXLabel(matrix3,tr("Word delimiters:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
  new FXTextField(matrix3,10,own,TextWindow::ID_DELIMITERS,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN,0,0,0,0, 2,2,1,1);


  //// File pattern settings button
  new FXButton(buttons,tr("Misc\tMiscellaneous\tMiscellaneous settings."),mis,switcher,FXSwitcher::ID_OPEN_FIFTH,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);

  // Bottom part
  new FXHorizontalSeparator(vertical,SEPARATOR_RIDGE|LAYOUT_FILL_X);
  FXHorizontalFrame *closebox=new FXHorizontalFrame(vertical,LAYOUT_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
  new FXButton(closebox,tr("&Accept"),NULL,this,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20);
  new FXButton(closebox,tr("&Cancel"),NULL,this,FXDialogBox::ID_CANCEL,BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20);
  }

/*******************************************************************************/

// Change patterns, each pattern separated by newline
void Preferences::setPatternList(const FXString& patterns){
  filepattext->setText(patterns);
  }


// Return list of patterns
FXString Preferences::getPatternList() const {
  return filepattext->getText();
  }


// Set language syntax
void Preferences::setSyntax(Syntax* syn){
  while(stylemat->getFirst()){ delete stylemat->getFirst(); }
  if(syn){
    FXCheckButton *check;
    FXColorWell   *well;
    FXLabel       *label;

    // Captions
    new FXLabel(stylemat,tr("Rule\tSyntax rule name"),NULL,JUSTIFY_LEFT|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Nfg\tNormal foreground color"),NULL,JUSTIFY_RIGHT|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Nbg\tNormal background color"),NULL,JUSTIFY_LEFT|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Sfg\tSelected background color"),NULL,JUSTIFY_RIGHT|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Sbg\tSelected background color"),NULL,JUSTIFY_LEFT|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Hfg\tHighlight background color"),NULL,JUSTIFY_RIGHT|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Hbg\tHighlight background color"),NULL,JUSTIFY_LEFT|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Act bg\tActive background color"),NULL,JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Und\tUnderline"),NULL,JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Str\tStrikeout"),NULL,JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
    new FXLabel(stylemat,tr("Bld\tBold face"),NULL,JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);

    // Add controls for each rule
    for(FXint i=0; i<syn->getNumRules()-1; i++){

      // Rule name
      label=new FXLabel(stylemat,syn->getRule(i+1)->getName(),NULL,JUSTIFY_LEFT|JUSTIFY_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
      label->setBackColor(getApp()->getBackColor());

      // Color wells
      well=new FXColorWell(stylemat,FXRGB(255,255,255),getOwner(),TextWindow::ID_STYLE_NORMAL_FG_FIRST+i,FRAME_LINE|LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN,0,0,28,18);
      well->setBackColor(getApp()->getBackColor());
      well=new FXColorWell(stylemat,FXRGB(255,255,255),getOwner(),TextWindow::ID_STYLE_NORMAL_BG_FIRST+i,FRAME_LINE|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN,0,0,28,18);
      well->setBackColor(getApp()->getBackColor());
      well=new FXColorWell(stylemat,FXRGB(255,255,255),getOwner(),TextWindow::ID_STYLE_SELECT_FG_FIRST+i,FRAME_LINE|LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN,0,0,28,18);
      well->setBackColor(getApp()->getBackColor());
      well=new FXColorWell(stylemat,FXRGB(255,255,255),getOwner(),TextWindow::ID_STYLE_SELECT_BG_FIRST+i,FRAME_LINE|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN,0,0,28,18);
      well->setBackColor(getApp()->getBackColor());
      well=new FXColorWell(stylemat,FXRGB(255,255,255),getOwner(),TextWindow::ID_STYLE_HILITE_FG_FIRST+i,FRAME_LINE|LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN,0,0,28,18);
      well->setBackColor(getApp()->getBackColor());
      well=new FXColorWell(stylemat,FXRGB(255,255,255),getOwner(),TextWindow::ID_STYLE_HILITE_BG_FIRST+i,FRAME_LINE|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN,0,0,28,18);
      well->setBackColor(getApp()->getBackColor());
      well=new FXColorWell(stylemat,FXRGB(255,255,255),getOwner(),TextWindow::ID_STYLE_ACTIVE_BG_FIRST+i,FRAME_LINE|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN,0,0,28,18);
      well->setBackColor(getApp()->getBackColor());

      // Style check buttons
      check=new FXCheckButton(stylemat,FXString::null,getOwner(),TextWindow::ID_STYLE_UNDERLINE_FIRST+i,LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,0,0, 0,0,0,0);
      check->setShadowColor(getApp()->getBorderColor());
      check->setHiliteColor(getApp()->getBorderColor());
      check->setBorderColor(getApp()->getBackColor());
      check->setBaseColor(getApp()->getBackColor());
      check=new FXCheckButton(stylemat,FXString::null,getOwner(),TextWindow::ID_STYLE_STRIKEOUT_FIRST+i,LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,0,0, 0,0,0,0);
      check->setShadowColor(getApp()->getBorderColor());
      check->setHiliteColor(getApp()->getBorderColor());
      check->setBorderColor(getApp()->getBackColor());
      check->setBaseColor(getApp()->getBackColor());
      check=new FXCheckButton(stylemat,FXString::null,getOwner(),TextWindow::ID_STYLE_BOLD_FIRST+i,LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,0,0, 0,0,0,0);
      check->setShadowColor(getApp()->getBorderColor());
      check->setHiliteColor(getApp()->getBorderColor());
      check->setBorderColor(getApp()->getBackColor());
      check->setBaseColor(getApp()->getBackColor());
      }
    }
  }


// Clean up
Preferences::~Preferences(){
  delete pal;
  delete ind;
  delete pat;
  delete sty;
  delete mis;
  }
