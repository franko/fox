/********************************************************************************
*                                                                               *
*                             S h e l l - C o m m a n d                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 2014,2023 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#ifndef SHELLCOMMAND_H
#define SHELLCOMMAND_H


class TextWindow;


// Shell Command
class ShellCommand : public FXObject {
  FXDECLARE(ShellCommand)
private:
  FXApp      *app;              // Application
  FXProcess   process;          // Child process
  FXString    directory;        // Directory where to start
  FXString    input;            // Input to child process
  FXPipe      ipipe;            // Pipe input to child
  FXPipe      opipe;            // Pipe output from child
  FXPipe      epipe;            // Pipe errors from child
  FXObject   *target;           // Target to notify
  FXSelector  selin;            // Message sent for input
  FXSelector  selout;           // Message sent for output
  FXSelector  selerr;           // Message sent for errors
  FXSelector  seldone;          // Message sent when done
private:
  ShellCommand(){}
  ShellCommand(const ShellCommand&);
  ShellCommand &operator=(const ShellCommand&);
public:
  long onCmdInput(FXObject*,FXSelector,void*);
  long onCmdOutput(FXObject*,FXSelector,void*);
  long onCmdError(FXObject*,FXSelector,void*);
public:
  enum {
    ID_INPUT=1,
    ID_OUTPUT,
    ID_ERROR
    };
public:

  // Construct shell command
  ShellCommand(FXApp* a,FXObject* tgt=nullptr,FXSelector so=0,FXSelector se=0,FXSelector sd=0);

  // Set directory
  void setDirectory(const FXString& dir){ directory=dir; }

  // Return directory
  const FXString& getDirectory() const { return directory; }

  // Set string as command input
  void setInput(const FXString& str);

  // Return input
  const FXString& getInput() const { return input; }

  // Access target
  void setTarget(FXObject* tgt){ target=tgt; }
  FXObject* getTarget() const { return target; }

  // Access input message
  void setInputMessage(FXSelector sel){ selin=sel; }
  FXSelector getInputMessage() const { return selin; }

  // Access output message
  void setOutputMessage(FXSelector sel){ selout=sel; }
  FXSelector getOutputMessage() const { return selout; }

  // Access error message
  void setErrorMessage(FXSelector sel){ selerr=sel; }
  FXSelector getErrorMessage() const { return selerr; }

  // Access done message
  void setDoneMessage(FXSelector sel){ seldone=sel; }
  FXSelector getDoneMessage() const { return seldone; }

  // Start command
  virtual FXbool start(const FXString& command);

  // Cancel command
  virtual FXbool cancel();

  // Stop command
  virtual FXbool stop();

  // Destroy it
  virtual ~ShellCommand();
  };

#endif
