/********************************************************************************
*                                                                               *
*                          P r o c e s s  -  T e s t                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 2010,2022 by Jeroen van der Zijp.   All Rights Reserved.        *
********************************************************************************/
#include "fx.h"


/*
  Notes:

  - Test process creation.

*/

/*******************************************************************************/



// Start
int main(int argc,char* argv[]){
  FXint code;

  // Trace
  setTraceLevel(151);

  // Make thread pool
  FXProcess process;

#if defined(WIN32)
  const FXchar command[]="C:\\Program Files\\Windows NT\\Accessories\\wordpad.exe";
  const FXchar *const args[]={command,"c:\\WINDOWS\\win.ini",nullptr};
#else
//  const FXchar command[]="/bin/ls";
//  const FXchar *const args[]={command,"-l","/usr/bin",nullptr};
  const FXchar command[]="/usr/local/bin/adie";
  const FXchar *const args[]={command,"process.cpp",nullptr};
#endif

  // Kick off
  if(!process.start(command,args)){
    fxwarning("failed to start: %s\n",command);
    return 1;
    }

  // Wait
  if(process.wait(code)){
    fxwarning("chiled exited with code: %d\n",code);
    }

  return 1;
  }

