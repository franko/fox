/********************************************************************************
*                                                                               *
*                          P r o c e s s  -  T e s t                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 2010,2021 by Jeroen van der Zijp.   All Rights Reserved.        *
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
  const FXchar *const args[]={command,"c:\\WINDOWS\\win.ini",NULL};
#else
//  const FXchar command[]="/bin/ls";
//  const FXchar *const args[]={command,"-l","/usr/bin",NULL};
  const FXchar command[]="/usr/local/bin/adie";
  const FXchar *const args[]={command,"process.cpp",NULL};
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

