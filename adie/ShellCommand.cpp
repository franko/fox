/********************************************************************************
*                                                                               *
*                             S h e l l - C o m m a n d                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 2014,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "Syntax.h"
#include "Modeline.h"
#include "TextWindow.h"
#include "Adie.h"
#include "ShellCommand.h"


/*
  Notes:

  - The ShellCommand object represents a running process inside Adie.

  - Need to add options to NOT capture output and stderr.

  - Need to add option to capture outputs into logger instead of text buffer.

  - On Windows, the pipe stuff won't work in addInput() because you can't
    block on anonymous pipes (or can you?).

  - FIXME add some code to break up commandline arguments, or alternatively,
    kick program off through shell (in this case, need to escape text to make
    magic characters safe from shell).

  - FIXME Maybe we should just connect child process's stderr to our own stderr?
*/

enum{BUFFERSIZE=2048};

/*******************************************************************************/

FXDEFMAP(ShellCommand) ShellCommandMap[]={
  FXMAPFUNC(SEL_IO_READ,ShellCommand::ID_OUTPUT,ShellCommand::onCmdOutput),
  FXMAPFUNC(SEL_IO_READ,ShellCommand::ID_LOGGER,ShellCommand::onCmdLogger),
  FXMAPFUNC(SEL_IO_WRITE,ShellCommand::ID_INPUT,ShellCommand::onCmdInput),
  };


FXIMPLEMENT(ShellCommand,FXObject,ShellCommandMap,ARRAYNUMBER(ShellCommandMap))


// Construct shell command
ShellCommand::ShellCommand(TextWindow* win,const FXString& dir,FXuint flgs):app(win->getApp()),window(win),selstartpos(0),selendpos(-1),selstartcol(0),selendcol(-1),directory(dir),ninput(0),noutput(0),flags(flgs){
  FXTRACE((1,"ShellCommand::ShellCommand(%p,%s,%x)\n",win,dir.text(),flgs));
  }


// Set selection
void ShellCommand::setSelection(FXint sp,FXint ep,FXint sc,FXint ec){
  selstartpos=sp;
  selendpos=ep;
  selstartcol=sc;
  selendcol=ec;
  }


// Execute command
FXbool ShellCommand::start(const FXString& command){
  FXTRACE((1,"ShellCommand::start(%s)\n",command.text()));
  FXbool result=false;
  if(!command.empty() && !process.id()){
    FXchar** argv=nullptr;

    // Assemble command
    if(FXPath::parseArgs(argv,command)){

      // Find the command (argv[0]) in the path
      FXString exec=FXPath::search(FXSystem::getExecPath(),argv[0]);

      // Got full path to executable
      if(!exec.empty()){

        // Old directory
        FXString cwd=FXSystem::getCurrentDirectory();

        // Switch to new directory
        if(FXSystem::setCurrentDirectory(directory)){

          // Pipes at child's end
          FXPipe ichild;
          FXPipe ochild;
          FXPipe echild;

          // Open pipe for child input (the parent writes, child reads)
          if(!pipe[0].open(ichild,FXIO::WriteOnly|FXIO::Inheritable)) goto x;

          // Open pipe for child outout (parent reads, child writes)
          if(!pipe[1].open(ochild,FXIO::ReadOnly|FXIO::Inheritable)) goto x;

          // Open pipe for child errors (parent reads, child writes)
          if(!pipe[2].open(echild,FXIO::ReadOnly|FXIO::Inheritable)) goto x;

          // Set handles to be used by child
          process.setInputStream(&ichild);
          process.setOutputStream(&ochild);
          process.setErrorStream(&echild);

          // Start it
          if(process.start(exec.text(),argv)){

            // Close child-side handles
            ichild.close();
            ochild.close();
            echild.close();

            // Set non-blocking on our end
            pipe[0].mode(pipe[0].mode()|FXIO::NonBlocking);
            pipe[1].mode(pipe[1].mode()|FXIO::NonBlocking);
            pipe[2].mode(pipe[2].mode()|FXIO::NonBlocking);

            // Set I/O callbacks
            if(pipe[0].isOpen()){
              getApp()->addInput(this,ID_INPUT,pipe[0].handle(),INPUT_WRITE);
              }
            if(pipe[1].isOpen()){
              getApp()->addInput(this,ID_OUTPUT,pipe[1].handle(),INPUT_READ);
              }
            if(pipe[2].isOpen()){
              getApp()->addInput(this,ID_LOGGER,pipe[2].handle(),INPUT_READ);
              }
            result=true;
            }

          // Switch back to old directory
          FXSystem::setCurrentDirectory(cwd);
          }
        }
x:    freeElms(argv);
      }
    }
  FXTRACE((1,"ShellCommand::start: %s\n",result?"OK":"FAILED"));
  return result;
  }


// Stop command
FXbool ShellCommand::stop(){
  FXTRACE((1,"ShellCommand::stop()\n"));
  if(process.id()){

    // Remove I/O callbacks
    if(pipe[0].isOpen()){
      getApp()->removeInput(pipe[0].handle(),INPUT_WRITE);
      pipe[0].close();
      }
    if(pipe[1].isOpen()){
      getApp()->removeInput(pipe[1].handle(),INPUT_READ);
      pipe[1].close();
      }
    if(pipe[2].isOpen()){
      getApp()->removeInput(pipe[2].handle(),INPUT_READ);
      pipe[2].close();
      }

    // Wait till child is done
    return process.wait();
    }
  return false;
  }


// Cancel command
FXbool ShellCommand::cancel(){
  FXTRACE((1,"ShellCommand::cancel()\n"));
  if(process.id()){
    process.kill();
    return stop();
    }
  return false;
  }

/*******************************************************************************/

// Input to child process
long ShellCommand::onCmdInput(FXObject*,FXSelector sel,void*){
  FXival count=pipe[0].writeBlock(&input[ninput],input.length()-ninput);
  if(count==FXIO::Again) return 1;
  if(0<=count){
    ninput+=count;
    if(count==0){
      getApp()->removeInput(pipe[0].handle(),INPUT_WRITE);
      pipe[0].close();
      }
    return 1;
    }
  return 1;
  }


// Output from child process
long ShellCommand::onCmdOutput(FXObject*,FXSelector sel,void*){
  FXchar buffer[BUFFERSIZE+1];
  FXival count=pipe[1].readBlock(buffer,BUFFERSIZE);
  FXTRACE((1,"ShellCommand::onCmdOutput: pipe[%d]: bytes: %ld\n",1,count));
  if(count==FXIO::Again) return 1;
  if(0<=count){
    buffer[count]='\0';
    if(flags&COLLECT){
      if(0<count){
        output.append(buffer,count);
        return 1;
        }
      if(flags&TO_LOG){
        getWindow()->logAppend(output);
        }
      else{
        getWindow()->textAppend(output);
        }
      }
    else{
      if(0<count){
        if(flags&TO_LOG){
          getWindow()->logAppend(buffer,count);
          }
        else{
          getWindow()->textAppend(buffer,count);
          }
        return 1;
        }
      }
    }
  getWindow()->doneCommand();
  return 1;
  }


// Logging from child process
// FIXME could use one color for stdout, and another for stderr.
long ShellCommand::onCmdLogger(FXObject*,FXSelector sel,void*){
  FXchar buffer[BUFFERSIZE+1];
  FXival count=pipe[2].readBlock(buffer,BUFFERSIZE);
  FXTRACE((1,"ShellCommand::onCmdLogger: pipe[%d]: bytes: %ld\n",2,count));
  if(count==FXIO::Again) return 1;
  if(0<=count){
    buffer[count]='\0';
    if(0<count){
      getWindow()->logAppend(buffer,count);
      return 1;
      }
    }
  return 1;
  }

/*******************************************************************************/

// Destroy shell command
ShellCommand::~ShellCommand(){
  FXTRACE((1,"ShellCommand::~ShellCommand\n"));
  stop();
  }

