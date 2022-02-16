/********************************************************************************
*                                                                               *
*                             String Format I/O Test                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 2007,2021 by Jeroen van der Zijp.   All Rights Reserved.        *
********************************************************************************/
#include "fx.h"
//#include <locale.h>
#include <time.h>


/*
  Notes:
  - Test battery for fxprintf().
*/


/*******************************************************************************/

namespace FX {

extern FXint __snprintf(FXchar* string,FXint length,const FXchar* format,...);
extern FXint __sscanf(const FXchar* string,const FXchar* format,...);

}

const FXchar *floatformat[]={
  "%.15e",
  "%'.5e",
  "%10.5f",
  "%-10.5f",
  "%+10.5f",
  "% 10.5f",
  "%123.9f",
  "%+22.9f",
  "%+4.9f",
  "%01.3f",
  "%4f",
  "%3.1f",
  "%3.2f",
  "%.0f",
  "%.3f",
  "%'.8f",
  "%+.3g",
  "%#.3g",
  "%.g",
  "%#.g",
  "%g",
  "%#g",
  "%'.8g",
  "%12.4e",
  "%a",
  "%A",
  "%+15.4a",
  };


const double floatnumbers[]={
  0.000000001,
  -1.5,
  0.8,
  1.0,                          // 0x1p+0
  10.0,
  100.0,
  1000.0,
  10000.0,
  999.0,
  1010.0,
  134.21,
  91340.2,
  341.1234,
  203.9,
  0.96,
  0.996,
  0.9996,
  1.996,
  4.136,
  6442452944.1234,
  1.23456789E+20,
  6.4969530541989433e-17,
  0.99999999,
  0.000009995,
  0.123456789,
  2.2250738585072014e-308,      // 0x1p-1022
  4.94065645841246544177e-324,  // 0x0.0000000000001p-1023
  1.7976931348623157e+308,      // 0x1.fffffffffffffp+1023
  1.9382023e-03,
  5e-320,
#if defined(__GNUC__) && !defined(WIN32)
  0x0.0000000002788p-1023,
#endif
  0.0,
  -0.0
  };


// Use a trick to get a nan
#if FOX_BIGENDIAN
const FXuint doublenan[2]={0x7fffffff,0xffffffff};
const FXuint doubleinf[2]={0x7ff00000,0x00000000};
#else
const FXuint doublenan[2]={0xffffffff,0x7fffffff};
const FXuint doubleinf[2]={0x00000000,0x7ff00000};
#endif


const FXchar *intformat[]={
  "%d",
  "%0d",
  "%'d",
  "%02x",
  "%0.2x",
  "%-8d",
  "%8d",
  "%08d",
  "%.6d",
  "%u",
  "%+i",
  "% i",
  "%x",
  "%#x",
  "%#08x",
  "%o",
  "%#o",
  "%.32b"
  };

const FXint intnumbers[]={
  0,
  1,
  -1,
  0x90,
  -34,
  2147483647,
  -2147483648
  };


const FXchar *positionalformat[]={
  "%d%d%d",
  "%3$d%2$d%1$d",
  "%2$*1$d%3$d"
  };

const FXchar *positionalformat2="%1$*2$.*3$lf";

const FXchar *positionalformat3="%3$d%3$d";


// Uncomment to revert to native version
//#define __snprintf snprintf

void specialcases(const char* fmt){
  FXchar buffer[1024];
  double num=1234567890.123456789;
  while(0.000001<=num){
    __snprintf(buffer,sizeof(buffer),fmt,num);
    fprintf(stdout,"format=\"%s\" output=\"%s\"\n",fmt,buffer);
    num*=0.1;
    }
  }

// Start
int main(int argc,char* argv[]){
  FXchar buffer[1024];
  FXuval x,y;

  //setlocale(LC_ALL,"");
  if(argc==2){
    FXdouble num=strtod(argv[1],NULL);
    __snprintf(buffer,sizeof(buffer),"%.20e",num);
    fprintf(stdout,"native: %.20e\n",num);
    fprintf(stdout,"ours  : %s\n",buffer);
    return 0;
    }

  // Testing int formats
  for(x=0; x<ARRAYNUMBER(intformat); x++){
    for(y=0; y<ARRAYNUMBER(intnumbers); y++){
      __snprintf(buffer,sizeof(buffer),intformat[x],intnumbers[y]);
      fprintf(stdout,"format=\"%s\" output=\"%s\"\n",intformat[x],buffer);
      }
    fprintf(stdout,"\n");
    }

  fprintf(stdout,"\n");

  // Testing double formats
  for(x=0; x<ARRAYNUMBER(floatformat); x++){
    for(y=0; y<ARRAYNUMBER(floatnumbers); y++){
      __snprintf(buffer,sizeof(buffer),floatformat[x],floatnumbers[y]);
      fprintf(stdout,"format=\"%s\" output=\"%s\"\n",floatformat[x],buffer);
      }
    fprintf(stdout,"\n");
    }

  for(y=0; y<ARRAYNUMBER(floatnumbers); y++){
    __snprintf(buffer,sizeof(buffer),"0x%016lx",Math::fpBits(floatnumbers[y]));
    fprintf(stdout,"format=\"%s\" output=\"%s\"\n","0x%016lx",buffer);
    }

  fprintf(stdout,"\n");

  // Testing Inf's
  for(x=0; x<ARRAYNUMBER(floatformat); x++){
    __snprintf(buffer,sizeof(buffer),floatformat[x],*((const FXdouble*)&doubleinf));
    fprintf(stdout,"format=\"%s\" output=\"%s\"\n",floatformat[x],buffer);
    }

  fprintf(stdout,"\n");

  // Testing NaN's
  for(x=0; x<ARRAYNUMBER(floatformat); x++){
    __snprintf(buffer,sizeof(buffer),floatformat[x],*((const FXdouble*)&doublenan));
    fprintf(stdout,"format=\"%s\" output=\"%s\"\n",floatformat[x],buffer);
    }

  fprintf(stdout,"\n");

  // Testing positional formats
  for(x=0; x<ARRAYNUMBER(positionalformat); x++){
    __snprintf(buffer,sizeof(buffer),positionalformat[x],10,20,30);
    fprintf(stdout,"format=\"%s\" output=\"%s\"\n",positionalformat[x],buffer);
    }

  fprintf(stdout,"\n");

  __snprintf(buffer,sizeof(buffer),positionalformat2,3.14159265358979,20,10);
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n",positionalformat2,buffer);

  __snprintf(buffer,sizeof(buffer),positionalformat3,10,20,30);
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n",positionalformat3,buffer);

  fprintf(stdout,"\n");

  // Special cases float formatting
  specialcases("%'.5e");
  specialcases("%'.5f");
  specialcases("%'.5g");

  __snprintf(buffer,sizeof(buffer),"%#.3g",0.0);
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n","%#.3g",buffer);

  fprintf(stdout,"\n");


  // Small dernormalized float, regular notation
  __snprintf(buffer,sizeof(buffer),"%.18le",5e-320);
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n","%.18le",buffer);
  __snprintf(buffer,sizeof(buffer),"%a",5e-320);
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n","%a",buffer);
  fprintf(stdout,"\n");

  // Small dernormalized float, passed as hex (types may be flagged as wrong)
  __snprintf(buffer,sizeof(buffer),"%.18le",FXULONG(0x0000000000002788));
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n","%.18le",buffer);
  __snprintf(buffer,sizeof(buffer),"%a",FXULONG(0x0000000000002788));
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n","%a",buffer);
  fprintf(stdout,"\n");

  // Francesco: disabled on windows, the test below fails on windows
  // using mingw64.
  // Small dernormalized float, passed as floating point hex syntax
#if defined(__GNUC__) && !defined(WIN32)
  __snprintf(buffer,sizeof(buffer),"%.18le",0x0.0000000002788p-1023);
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n","%.18le",buffer);
  __snprintf(buffer,sizeof(buffer),"%a",0x0.0000000002788p-1023);
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n","%a",buffer);
  fprintf(stdout,"\n");
#endif

  __snprintf(buffer,sizeof(buffer),"%10.5lf",6.4969530541989433e-17);
  fprintf(stdout,"format=\"%s\" output=\"%s\"\n","%10.5lf",buffer);

  return 0;
  }

