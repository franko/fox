/********************************************************************************
*                                                                               *
*                                   Dictionary Test                             *
*                                                                               *
*********************************************************************************
* Copyright (C) 2008,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
********************************************************************************/
#include "fx.h"

/*
  Notes:
  - Tests for FXDict.
*/

static inline FXTime timesource(){
  return FXThread::ticks();
//  return FXThread::time();
  }


int main(int argc,char* argv[]){
  FXTime beg,end,tot,max,ins;
  FXchar buf[1000];
  FXDictionary dict;

  max=0;
  tot=0;
  ins=0;

  // Insertions
  while(fgets(buf,sizeof(buf),stdin)){
    if(strchr(buf,'\n')) *strchr(buf,'\n')='\0';

    beg=timesource();
    dict.insert(buf,nullptr);
    end=timesource();
    tot+=(end-beg);
    max=(end-beg)>max?(end-beg):max;
    ins++;
    }

  printf("insertions: %lld avg: %lld max: %lld\n",ins,tot/ins,max);


  max=0;
  tot=0;
  ins=0;
  rewind(stdin);

  // Lookups
  while(fgets(buf,sizeof(buf),stdin)){
    if(strchr(buf,'\n')) *strchr(buf,'\n')='\0';

    beg=timesource();
    dict.at(buf);
    end=timesource();
    tot+=(end-beg);
    max=(end-beg)>max?(end-beg):max;
    ins++;
    }

  printf("Lookups: %lld avg: %lld max: %lld\n",ins,tot/ins,max);

  max=0;
  tot=0;
  ins=0;
  rewind(stdin);

  // Removes
  while(fgets(buf,sizeof(buf),stdin)){
    if(strchr(buf,'\n')) *strchr(buf,'\n')='\0';

    beg=timesource();
    dict.remove(buf);
    end=timesource();
    tot+=(end-beg);
    max=(end-beg)>max?(end-beg):max;
    ins++;
    }

  printf("Removes: %lld avg: %lld max: %lld\n",ins,tot/ins,max);
  printf("no=%d used=%d\n",dict.no(),dict.no());

#if 0
  for(int i=0; i<dict.no(); ++i){
    if(!dict.empty(i)){
      printf("%d %s\n",i,dict.key(i).text());
      }
    }
#endif

  return 0;
  }

