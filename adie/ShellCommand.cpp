/********************************************************************************
*                                                                               *
*                             S h e l l - C o m m a n d                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 2014,2021 by Jeroen van der Zijp.   All Rights Reserved.        *
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
  - FIXME add some code to break up commandline arguments, or alternatively,
    kick program off through shell (in this case, need to escape text to make
    magic characters safe from shell).
*/

/*******************************************************************************/

FXDEFMAP(ShellCommand) ShellCommandMap[]={
  FXMAPFUNC(SEL_IO_READ,ShellCommand::ID_ERROR,ShellCommand::onCmdError),
  FXMAPFUNC(SEL_IO_READ,ShellCommand::ID_OUTPUT,ShellCommand::onCmdOutput),
  FXMAPFUNC(SEL_IO_WRITE,ShellCommand::ID_INPUT,ShellCommand::onCmdInput),
  };


FXIMPLEMENT(ShellCommand,FXObject,ShellCommandMap,ARRAYNUMBER(ShellCommandMap))


// Construct shell command
ShellCommand::ShellCommand(FXApp* a,FXObject* tgt,FXSelector so,FXSelector se,FXSelector sd):app(a),target(tgt),selout(so),selerr(se),seldone(sd){
  FXTRACE((1,"ShellCommand::ShellCommand\n"));
  }


// Set string as command input
void ShellCommand::setInput(const FXString& str){
  input=str;
  }


// Execute command
FXbool ShellCommand::start(const FXString& command){
  FXTRACE((1,"ShellCommand::start(%s)\n",command.text()));
  FXbool result=false;
  if(!command.empty() && !process.id()){
    FXchar** argv;

    // Assemble command
    if(FXPath::parseArgs(argv,command)){

      // Try find the command (argv[0]) in the path
      FXString exec=FXPath::search(FXSystem::getExecPath(),argv[0]);
      if(!exec.empty()){

        // Pipes at child's end
        FXPipe ichild;
        FXPipe ochild;
        FXPipe echild;

        // Open pipe for child input (the parent writes, child reads)
        if(!ipipe.open(ichild,FXIO::WriteOnly|FXIO::Inheritable)) return false;

        // Open pipe for child outout (parent reads, child writes)
        if(!opipe.open(ochild,FXIO::ReadOnly|FXIO::Inheritable)) return false;

        // Open pipe for child errors (parent reads, child writes)
        if(!epipe.open(echild,FXIO::ReadOnly|FXIO::Inheritable)) return false;

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
          ipipe.setMode(ipipe.mode()|FXIO::NonBlocking);
          opipe.setMode(opipe.mode()|FXIO::NonBlocking);
          epipe.setMode(epipe.mode()|FXIO::NonBlocking);

          // Set I/O callbacks
          if(ipipe.isOpen()){
            app->addInput(this,ID_INPUT,ipipe.handle(),INPUT_WRITE);
            }
          if(opipe.isOpen()){
            app->addInput(this,ID_OUTPUT,opipe.handle(),INPUT_READ);
            }
          if(epipe.isOpen()){
            app->addInput(this,ID_ERROR,epipe.handle(),INPUT_READ);
            }
          result=true;
          }
        }
      freeElms(argv);
      }
    }
  return result;
  }


// Input to command
long ShellCommand::onCmdInput(FXObject*,FXSelector,void*){
  FXTRACE((1,"ShellCommand::onCmdInput\n"));
  FXival count;
  count=ipipe.writeBlock(input.text(),input.length());
  if(count<0){
    if(count==FXIO::Broken){
      app->removeInput(ipipe.handle(),INPUT_WRITE);
      ipipe.close();
      return 1;
      }
    if(count!=FXIO::Again){
      if(target && seldone){ target->handle(this,seldone,NULL); }
      stop();
      }
    return 1;
    }
  if(count==input.length()){
    app->removeInput(ipipe.handle(),INPUT_WRITE);
    ipipe.close();
    return 1;
    }
  input.erase(0,count);
  return 1;
  }


// Output from command
long ShellCommand::onCmdOutput(FXObject*,FXSelector,void*){
  FXTRACE((1,"ShellCommand::onCmdOutput\n"));
  FXchar buffer[2048];
  FXival count;
  count=opipe.readBlock(buffer,sizeof(buffer)-1);
  if(count<0){
    if(count!=FXIO::Again){
      stop();
      if(target && seldone){ target->handle(this,seldone,NULL); }
      }
    return 1;
    }
  if(count==0){
    app->removeInput(opipe.handle(),INPUT_READ);
    opipe.close();
    if(!epipe.isOpen()){
      stop();
      if(target && seldone){ target->handle(this,seldone,NULL); }
      }
    return 1;
    }
  buffer[count]='\0';
  if(target && selout){ target->handle(this,selout,buffer); }
  return 1;
  }



// Errors from command
long ShellCommand::onCmdError(FXObject*,FXSelector,void*){
  FXTRACE((1,"ShellCommand::onCmdError\n"));
  FXchar buffer[2048];
  FXival count;
  count=epipe.readBlock(buffer,sizeof(buffer)-1);
  if(count<0){
    if(count!=FXIO::Again){
      stop();
      if(target && seldone){ target->handle(this,seldone,NULL); }
      }
    return 1;
    }
  if(count==0){
    app->removeInput(epipe.handle(),INPUT_READ);
    epipe.close();
    if(!opipe.isOpen()){
      stop();
      if(target && seldone){ target->handle(this,seldone,NULL); }
      }
    return 1;
    }
  buffer[count]='\0';
  if(target && selerr){ target->handle(this,selerr,buffer); }
  return 1;
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


// Stop command
FXbool ShellCommand::stop(){
  FXTRACE((1,"ShellCommand::stop()\n"));
  if(process.id()){

    // Remove I/O callbacks
    if(ipipe.isOpen()){
      app->removeInput(ipipe.handle(),INPUT_WRITE);
      ipipe.close();
      }
    if(opipe.isOpen()){
      app->removeInput(opipe.handle(),INPUT_READ);
      opipe.close();
      }
    if(epipe.isOpen()){
      app->removeInput(epipe.handle(),INPUT_READ);
      epipe.close();
      }

    // Wait till child is done
    return process.wait();
    }
  return false;
  }


// Destroy shell command
ShellCommand::~ShellCommand(){
  FXTRACE((1,"ShellCommand::~ShellCommand\n"));
  stop();
  }

