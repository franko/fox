/********************************************************************************
*                                                                               *
*                             Regular Expression Test                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2022 by Jeroen van der Zijp.   All Rights Reserved.        *
********************************************************************************/
#include "fx.h"


/*


*/

#define NCAP 10    // Must be less that or equal to 10


/*******************************************************************************/


// Print command line help
static void printusage(){
  fxmessage("Usage: rex [options] pat [string]\n");
  fxmessage("  options:\n");
  fxmessage("  -?, -h, --help                      Print help.\n");
  fxmessage("  -c, --capture                       Capturing parentheses.\n");
  fxmessage("  -i, --ignore-case                   Case-insensitive.\n");
  fxmessage("  -n, --not-empty                     Match should be non-empty.\n");
  fxmessage("  -x, --exact                         Match should consume all of string.\n");
  fxmessage("  -r, --reverse                       Reverse match.\n");
  fxmessage("  -nb, --not-bol                      Start of string is not begin of line.\n");
  fxmessage("  -ne, --not-eol                      End of string is not end of line.\n");
  fxmessage("  -u, --unicode                       Unicode mode.\n");
  fxmessage("  -v, --verbatim                      Verbatim mode.\n");
  fxmessage("  -l <num>, --levels <num>            Capure levels.\n");
  }


// Start the whole thing
int main(int argc,char** argv){
  FXint beg[NCAP]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  FXint end[NCAP]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  FXint mode=FXRex::Normal;
  FXint ncap=1;
  FXint where;
  FXint arg=1;
  FXRex rex;
  FXRex::Error err;

  setTraceLevel(100);

  // Parse options
  while(arg<argc && argv[arg][0]=='-'){
    if(compare(argv[arg],"-c")==0 || compare(argv[arg],"--capture")==0){
      mode|=FXRex::Capture;
      }
    else if(compare(argv[arg],"-i")==0 || compare(argv[arg],"--ignore-case")==0){
      mode|=FXRex::IgnoreCase;
      }
    else if(compare(argv[arg],"-n")==0 || compare(argv[arg],"--not-empty")==0){
      mode|=FXRex::NotEmpty;
      }
    else if(compare(argv[arg],"-x")==0 || compare(argv[arg],"--exact")==0){
      mode|=FXRex::Exact;
      }
    else if(compare(argv[arg],"-r")==0 || compare(argv[arg],"--reverse")==0){
      mode|=FXRex::Reverse;
      }
    else if(compare(argv[arg],"-b")==0 || compare(argv[arg],"--not-bol")==0){
      mode|=FXRex::NotBol;
      }
    else if(compare(argv[arg],"-e")==0 || compare(argv[arg],"--not-eol")==0){
      mode|=FXRex::NotEol;
      }
    else if(compare(argv[arg],"-u")==0 || compare(argv[arg],"--unicode")==0){
      mode|=FXRex::Unicode;
      }
    else if(compare(argv[arg],"-v")==0 || compare(argv[arg],"--verbatim")==0){
      mode|=FXRex::Verbatim;
      }
    else if(compare(argv[arg],"-l")==0 || compare(argv[arg],"--levels")==0){
      if(++arg>=argc){ fxwarning("rex: missing capture levels.\n"); return 1; }
      sscanf(argv[arg],"%d",&ncap);
      }
    else if(compare(argv[arg],"-?")==0 || compare(argv[arg],"-h")==0 || compare(argv[arg],"--help")==0){
      printusage();
      return 0;
      }
    arg++;
    }

  // Pattern
  if(arg<argc){

    // Parse expression
    err=rex.parse(argv[arg],mode);

    // Show result of parse
    fxmessage("parse(\"%s\") = %s\n",argv[arg],FXRex::getError(err));

    // Next
    arg++;

    // Input
    if(arg<argc){

      // Search string with pattern
      where=rex.search(argv[arg],strlen(argv[arg]),0,strlen(argv[arg]),mode,beg,end,ncap);

      // If found, show where
      if(0<=where){
        fxmessage("found at %d\n",where);
        for(FXint i=0; i<ncap; i++){
          fxmessage("capture at %d:%d\n",beg[i],end[i]);
          }
        for(FXint i=beg[0]; i<end[0]; i++){
          fxmessage("%c",argv[arg][i]);
          }
        fxmessage("\n");
        }
      else{
        fxmessage("no match\n");
        }

      // Next
      arg++;
      }
    }
  return 1;
  }

