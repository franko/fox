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
#ifndef SHELLCOMMAND_H
#define SHELLCOMMAND_H



// Shell Command
class ShellCommand : public FXObject {
  FXDECLARE(ShellCommand)
private:
  FXApp          *app;          // Application
  TextWindow     *window;       // Window to send messages to
  FXProcess       process;      // Child process
  FXPipe          pipe[3];      // Pipes to communicate with child {in,out,err}
  FXint           selstartpos;  // Start of selection
  FXint           selendpos;    // End of selection
  FXint           selstartcol;  // Start column
  FXint           selendcol;    // End column
  FXString        directory;    // Directory where to start
  FXString        input;        // Input to child process
  FXString        output;       // Output from child process
  FXival          ninput;       // Number of inputs sent to child
  FXival          noutput;      // Number of outputs received from child
  FXuint          flags;        // Flags
private:
  ShellCommand(){}
  ShellCommand(const ShellCommand&);
  ShellCommand &operator=(const ShellCommand&);
public:
  long onCmdInput(FXObject*,FXSelector,void*);
  long onCmdOutput(FXObject*,FXSelector,void*);
  long onCmdLogger(FXObject*,FXSelector,void*);
public:

  // Handler ids
  enum {
    ID_INPUT  = 1,
    ID_OUTPUT = 2,
    ID_LOGGER = 3
    };

public:

  // Collect or pass on
  enum {
    STREAM =0,
    COLLECT=1,
    };

  // Command Flags
  enum{
    SAVE_DOC=2,         // Save document before
    LOAD_DOC=4,         // Load back after
    };

  // Command Input options
  enum{
    FROM_SEL=8,                   // Input from selection
    FROM_DOC=16,                  // Input from document
    FROM_ANY=(FROM_SEL|FROM_DOC), // Input from document or selection
    };

  // Command Output options
  enum{
    TO_INS=16,                  // Output inserted at cursor
    TO_REP=32,                  // Output replaces input
    TO_LOG=64,                  // Output to log window
    TO_NEW=128                  // Output to new document window
    };
public:

  // Construct shell command
  ShellCommand(TextWindow* win,const FXString& dir,FXuint flgs=STREAM);

  // Get application
  FXApp* getApp() const { return app; }

  // Get window
  TextWindow* getWindow() const { return window; }

  // Set directory
  void setDirectory(const FXString& dir){ directory=dir; }

  // Return directory
  const FXString& getDirectory() const { return directory; }

  // Return flags
  FXuint getFlags() const { return flags; }

  // Set selection
  void setSelection(FXint sp=0,FXint ep=-1,FXint sc=0,FXint ec=-1);

  // Return selection start position
  FXint getSelStartPos() const { return selstartpos; }

  // Return selection end position
  FXint getSelEndPos() const { return selendpos; }

  // Return selection start column
  FXint getSelStartColumn() const { return selstartcol; }

  // Return selection end column
  FXint getSelEndColumn() const { return selendcol; }

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
