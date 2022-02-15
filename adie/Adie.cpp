/********************************************************************************
*                                                                               *
*                     T h e   A d i e   T e x t   E d i t o r                   *
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
#include "fxkeys.h"
#include <signal.h>
#ifndef WIN32
#include <sys/wait.h>
#endif
#include "HelpWindow.h"
#include "Preferences.h"
#include "Commands.h"
#include "Modeline.h"
#include "Syntax.h"
#include "SyntaxParser.h"
#include "TextWindow.h"
#include "FindInFiles.h"
#include "Adie.h"
#include "icons.h"

/*
  Notes:
  - One single collection of icons.
  - Manage list of open windows.
*/

/*******************************************************************************/


// Map
FXDEFMAP(Adie) AdieMap[]={
  FXMAPFUNC(SEL_SIGNAL,Adie::ID_HARVEST,Adie::onSigHarvest),
  FXMAPFUNC(SEL_SIGNAL,Adie::ID_CLOSEALL,Adie::onCmdCloseAll),
  FXMAPFUNC(SEL_COMMAND,Adie::ID_CLOSEALL,Adie::onCmdCloseAll),
  FXMAPFUNC(SEL_COMMAND,Adie::ID_SYNTAXPATHS,Adie::onCmdSyntaxPaths),
  FXMAPFUNC(SEL_UPDATE,Adie::ID_SYNTAXPATHS,Adie::onUpdSyntaxPaths),
  };


// Object implementation
FXIMPLEMENT(Adie,FXApp,AdieMap,ARRAYNUMBER(AdieMap))


// Make some windows
Adie::Adie(const FXString& name):FXApp(name){

  // Make some icons; these are shared between all text windows
  bigicon=new FXGIFIcon(this,big_gif);
  smallicon=new FXGIFIcon(this,small_gif);
  newicon=new FXGIFIcon(this,new_gif,0,IMAGE_ALPHAGUESS);
  reloadicon=new FXGIFIcon(this,reload_gif);
  openicon=new FXGIFIcon(this,open_gif);
  saveicon=new FXGIFIcon(this,save_gif);
  saveasicon=new FXGIFIcon(this,saveas_gif,0,IMAGE_ALPHAGUESS);
  savetoicon=new FXGIFIcon(this,saveto_gif,0,IMAGE_ALPHAGUESS);
  printicon=new FXGIFIcon(this,print_gif);
  cuticon=new FXGIFIcon(this,cut_gif);
  copyicon=new FXGIFIcon(this,copy_gif);
  pasteicon=new FXGIFIcon(this,paste_gif);
  deleteicon=new FXGIFIcon(this,delete_gif);
  undoicon=new FXGIFIcon(this,undo_gif);
  redoicon=new FXGIFIcon(this,redo_gif);
  fontsicon=new FXGIFIcon(this,fonts_gif);
  helpicon=new FXGIFIcon(this,help_gif);
  quiticon=new FXGIFIcon(this,quit_gif);
  searchicon=new FXGIFIcon(this,search_gif,0,IMAGE_ALPHAGUESS);
  replaceicon=new FXGIFIcon(this,replace_gif,0,IMAGE_ALPHAGUESS);
  searchnexticon=new FXGIFIcon(this,searchnext_gif,0,IMAGE_ALPHAGUESS);
  searchprevicon=new FXGIFIcon(this,searchprev_gif,0,IMAGE_ALPHAGUESS);
  bookseticon=new FXGIFIcon(this,bookset_gif);
  booknexticon=new FXGIFIcon(this,booknext_gif);
  bookprevicon=new FXGIFIcon(this,bookprev_gif);
  bookdelicon=new FXGIFIcon(this,bookdel_gif);
  shiftlefticon=new FXGIFIcon(this,shiftleft_gif);
  shiftrighticon=new FXGIFIcon(this,shiftright_gif);
  configicon=new FXGIFIcon(this,config_gif);
  browsericon=new FXGIFIcon(this,browser);
  nobrowsericon=new FXGIFIcon(this,nobrowser);
  loggericon=new FXGIFIcon(this,logger);
  nologgericon=new FXGIFIcon(this,nologger);
  uppercaseicon=new FXGIFIcon(this,uppercase);
  lowercaseicon=new FXGIFIcon(this,lowercase);
  backwardicon=new FXGIFIcon(this,backward_gif);
  forwardicon=new FXGIFIcon(this,forward_gif);
  shownicon=new FXGIFIcon(this,fileshown);
  hiddenicon=new FXGIFIcon(this,filehidden);

#ifndef DEBUG
  // If interrupt happens, quit gracefully; we may want to
  // save edit buffer contents w/o asking if display gets
  // disconnected or if hangup signal is received.
  addSignal(SIGINT,this,ID_CLOSEALL);
#ifndef WIN32
  addSignal(SIGQUIT,this,ID_CLOSEALL);
  addSignal(SIGHUP,this,ID_CLOSEALL);
  addSignal(SIGPIPE,this,ID_CLOSEALL);
#endif
#endif

#ifndef WIN32
  // On unix, we need to catch SIGCHLD to harvest zombie child processes.
  //addSignal(SIGCHLD,this,ID_HARVEST,true);
#endif

  // File associations, shared between all windows
  associations=new FXFileAssociations(this);
  }


// Close all windows
long Adie::onCmdCloseAll(FXObject*,FXSelector,void*){
  while(0<windowlist.no() && windowlist[0]->close(true)){}
  return 1;
  }


// Change syntax paths
long Adie::onCmdSyntaxPaths(FXObject* sender,FXSelector,void*){
  sender->handle(this,FXSEL(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE),(void*)&syntaxpaths);
  reg().writeStringEntry("SETTINGS","syntaxpaths",syntaxpaths.text());
  return 1;
  }


// Update syntax paths
long Adie::onUpdSyntaxPaths(FXObject* sender,FXSelector,void*){
  sender->handle(this,FXSEL(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE),(void*)&syntaxpaths);
  return 1;
  }


// Harvest the zombies :-)
long Adie::onSigHarvest(FXObject*,FXSelector,void*){
  fxmessage("Harvesting...\n");
#ifndef WIN32
  while(waitpid(-1,NULL,WNOHANG)>0){ }
#endif
  return 1;
  }


/*******************************************************************************/

// Print command line help
static void printusage(){
  fxmessage("Usage: adie [options] files...\n");
  fxmessage("  options:\n");
  fxmessage("  -?, -h, --help                      Print help.\n");
  fxmessage("  -V, --version                       Print version number.\n");
  fxmessage("  -v, --view                          Start in view-only mode.\n");
  fxmessage("  -e, --edit                          Start in edit-mode.\n");
  fxmessage("  -l NUM, --line NUM                  Jump cursor position to line number.\n");
  fxmessage("  -c NUM, --col NUM                   Jump cursor position to column.\n");
  fxmessage("  -S SYNTAXFILE, --syntax SYNTAXFILE  Load given syntax file.\n");
  fxmessage("  -L LANGUAGE, --lang LANGUAGE        Force language mode.\n");
  }


// Print verson info
static void printversion(){
  fxmessage("A.d.i.e. - ADvanced Interactive Editor %d.%d.%d.\n",VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH);
  fxmessage("Copyright (C) 2000,2020 Jeroen van der Zijp.  All Rights Reserved.\n\n");
  fxmessage("Please visit: http://www.fox-toolkit.org for further information.\n");
  fxmessage("\n");
  fxmessage("This program is free software: you can redistribute it and/or modify\n");
  fxmessage("it under the terms of the GNU General Public License as published by\n");
  fxmessage("the Free Software Foundation, either version 3 of the License, or\n");
  fxmessage("(at your option) any later version.\n");
  fxmessage("\n");
  fxmessage("This program is distributed in the hope that it will be useful,\n");
  fxmessage("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
  fxmessage("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
  fxmessage("GNU General Public License for more details.\n");
  fxmessage("\n");
  fxmessage("You should have received a copy of the GNU General Public License\n");
  fxmessage("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
  }


// Start the application
FXint Adie::start(int argc,char** argv){
  FXString    file,lang,execpath,iconpath,syntaxfile;
  TextWindow *window=NULL;
  Syntax     *syntax=NULL;
  FXbool      edit=true;
  FXint       line=0;
  FXint       col=0;
  FXint       arg=1;

  // The registry has been loaded after this
  init(argc,argv);

  // Make a tool tip
  new FXToolTip(this,0);

  // Create it
  create();

  // Exec path is default for syntax path
  execpath=FXSystem::getExecPath();

  FXTRACE((10,"execpath=%s\n",execpath.text()));

  // See if override paths are provided in the registry
  syntaxpaths=reg().readStringEntry("SETTINGS","syntaxpaths",execpath.text());

  FXTRACE((10,"syntaxpaths=%s\n",syntaxpaths.text()));

  // Look for syntax file in the syntax paths
  syntaxfile=FXPath::search(syntaxpaths,"Adie.stx");

  FXTRACE((10,"syntaxfile=%s\n",syntaxfile.text()));

  // Get icon search path
  iconpath=reg().readStringEntry("SETTINGS","iconpath",FXIconCache::defaultIconPath);

  FXTRACE((10,"iconpath=%s\n",iconpath.text()));

  // Change icon search path
  associations->setIconPath(iconpath);

  // Parse options first
  while(arg<argc && argv[arg][0]=='-'){
    if(compare(argv[arg],"-v")==0 || compare(argv[arg],"--view")==0){
      edit=false;
      }
    else if(compare(argv[arg],"-e")==0 || compare(argv[arg],"--edit")==0){
      edit=true;
      }
    else if(compare(argv[arg],"-?")==0 || compare(argv[arg],"-h")==0 || compare(argv[arg],"--help")==0){
      printusage();
      return 0;
      }
    else if(compare(argv[arg],"-V")==0 || compare(argv[arg],"--version")==0){
      printversion();
      return 0;
      }
    else if(compare(argv[arg],"-l")==0 || compare(argv[arg],"--line")==0){
      if(++arg>=argc){ fxwarning("Adie: missing line number.\n"); return 1; }
      sscanf(argv[arg],"%d",&line);
      }
    else if(compare(argv[arg],"-c")==0 || compare(argv[arg],"--col")==0){
      if(++arg>=argc){ fxwarning("Adie: missing column number.\n"); return 1; }
      sscanf(argv[arg],"%d",&col);
      }
    else if(compare(argv[arg],"-S")==0 || compare(argv[arg],"--syntax")==0){
      if(++arg>=argc){ fxwarning("Adie: missing syntax file.\n"); return 1; }
      syntaxfile=argv[arg];
      }
    else if(compare(argv[arg],"-L")==0 || compare(argv[arg],"--lang")==0){
      if(++arg>=argc){ fxwarning("Adie: missing language mode.\n"); return 1; }
      lang=argv[arg];
      }
    else{
      fxwarning("Adie: unknown command line argument.\n");
      return 1;
      }
    arg++;
    }

  // Load syntax file
  if(!syntaxfile.empty()){
    if(!SyntaxParser::parseFile(syntaxes,syntaxfile)){
      fxwarning("Adie: unable to parse syntax file: %s.\n",syntaxfile.text());
      }
    }

  // Get syntax
  syntax=getSyntaxByName(lang);

  // Parse filenames
  while(arg<argc){

    // Make new window
    window=new TextWindow(this);
    window->create();

    // Compute absolute path
    file=FXPath::absolute(argv[arg]);

    // Start in directory with empty untitled file
    if(FXStat::isDirectory(file)){
      file=unique(file);
      window->setFilename(file);
      window->setFilenameSet(false);
      window->setBrowserCurrentFile(file);
      }

    // Start in directory with existing, accessible file
    else if(FXStat::isFile(file) && window->loadFile(file)){
      window->readBookmarks(file);
      window->readView(file);
      window->setEditable(edit);
      window->determineSyntax();
      window->parseModeline();
      if(line) window->visitLine(line,col);
      }

    // Start in directory with empty or inaccessible file
    else{
      window->setFilename(file);
      window->setFilenameSet(false);    // Prompt for name when saving
      window->determineSyntax();
      window->setBrowserCurrentFile(file);
      }

    // Override language mode?
    if(syntax){
      window->setSyntax(syntax);
      }
    arg++;
    }

  // Start in current directory with empty untitled file
  if(!window){

    // New window
    window=new TextWindow(this);
    window->create();

    // Compute absolute path
    file=FXPath::absolute("untitled");
    window->setFilename(file);
    window->setFilenameSet(false);
    window->setBrowserCurrentFile(file);

    // Override language mode?
    if(syntax){
      window->setSyntax(syntax);
      }
    }

  // Now run
  return run();
  }


// Generate unique name from given path
FXString Adie::unique(const FXString& path) const {
  FXString name="untitled";
  FXString file;
  for(FXint i=1; i<2147483647; i++){
    file=FXPath::absolute(path,name);
    if(!findWindow(file)) break;
    name.format("untitled%d",i);
    }
  return file;
  }


// Find an as yet untitled, unedited window
TextWindow *Adie::findUnused() const {
  for(FXint w=0; w<windowlist.no(); w++){
    if(!windowlist[w]->isFilenameSet() && !windowlist[w]->isModified()){
      return windowlist[w];
      }
    }
  return NULL;
  }


// Find window, if any, currently editing the given file
TextWindow* Adie::findWindow(const FXString& file) const {
  for(FXint w=0; w<windowlist.no(); w++){
    if(windowlist[w]->getFilename()==file){
      return windowlist[w];
      }
    }
  return NULL;
  }


// Open file and jump to line, or just jump to line if already open
TextWindow* Adie::openFileWindow(const FXString& file,FXint lineno,FXint column){
  TextWindow *window=NULL;

  FXTRACE((1,"Adie::openFileWindow(%s,%d,%d)\n",file.text(),lineno,column));

  // See if we already have this file
  window=findWindow(file);
  if(!window){

    // Create new one if no unused windows
    window=findUnused();
    if(!window){
      window=new TextWindow(this);
      window->create();
      }

    // Load the file
    if(window->loadFile(file)){
      window->readBookmarks(file);
      window->readView(file);
      window->determineSyntax();
      window->parseModeline();
      }
    }

  // Switch line number only
  if(lineno){
    window->visitLine(lineno,column);
    }

  // Bring up the window
  window->raise();
  window->setFocus();
  return window;
  }


// Get syntax for language name
Syntax* Adie::getSyntaxByName(const FXString& lang){
  FXTRACE((10,"Adie::getSyntaxByName(%s)\n",lang.text()));
  if(!lang.empty()){
    for(FXint syn=0; syn<syntaxes.no(); syn++){
      if(syntaxes[syn]->getName()==lang){
        FXTRACE((10,"syntaxes[%d]: language: %s matched name: %s!\n",syn,syntaxes[syn]->getName().text(),lang.text()));
        return syntaxes[syn];
        }
      }
    }
  return NULL;
  }


// Get syntax by consulting registry
Syntax* Adie::getSyntaxByRegistry(const FXString& file){
  FXTRACE((10,"Adie::getSyntaxByRegistry(%s)\n",file.text()));
  if(!file.empty()){
    FXString name=FXPath::name(file);
    FXString lang=reg().readStringEntry("SYNTAX",name);
    return getSyntaxByName(lang);
    }
  return NULL;
  }


// Get syntax by matching file patterns
Syntax* Adie::getSyntaxByPattern(const FXString& file){
  FXTRACE((10,"Adie::getSyntaxByPattern(%s)\n",file.text()));
  if(!file.empty()){
    for(FXint syn=0; syn<syntaxes.no(); syn++){
      if(syntaxes[syn]->matchFilename(file)){
        FXTRACE((10,"syntaxes[%d]: language: %s matched file: %s!\n",syn,syntaxes[syn]->getName().text(),file.text()));
        return syntaxes[syn];
        }
      }
    }
  return NULL;
  }


// Get syntax by matching file contents
Syntax* Adie::getSyntaxByContents(const FXString& contents){
  FXTRACE((10,"Adie::getSyntaxByContents(%s)\n",contents.text()));
  if(!contents.empty()){
    for(FXint syn=0; syn<syntaxes.no(); syn++){
      if(syntaxes[syn]->matchContents(contents)){
        FXTRACE((10,"syntaxes[%d]: language: %s matched contents: %s!\n",syn,syntaxes[syn]->getName().text(),contents.text()));
        return syntaxes[syn];
        }
      }
    }
  return NULL;
  }


/*******************************************************************************/

// Clean up the mess
Adie::~Adie(){
  for(int i=0; i<syntaxes.no(); i++) delete syntaxes[i];
  FXASSERT(windowlist.no()==0);
  delete associations;
  delete bigicon;
  delete smallicon;
  delete newicon;
  delete reloadicon;
  delete openicon;
  delete saveicon;
  delete saveasicon;
  delete savetoicon;
  delete printicon;
  delete cuticon;
  delete copyicon;
  delete pasteicon;
  delete deleteicon;
  delete undoicon;
  delete redoicon;
  delete fontsicon;
  delete helpicon;
  delete quiticon;
  delete searchicon;
  delete replaceicon;
  delete searchnexticon;
  delete searchprevicon;
  delete bookseticon;
  delete booknexticon;
  delete bookprevicon;
  delete bookdelicon;
  delete shiftlefticon;
  delete shiftrighticon;
  delete configicon;
  delete browsericon;
  delete nobrowsericon;
  delete loggericon;
  delete nologgericon;
  delete uppercaseicon;
  delete lowercaseicon;
  delete backwardicon;
  delete forwardicon;
  delete shownicon;
  delete hiddenicon;
  }

