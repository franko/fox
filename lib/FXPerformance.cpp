/********************************************************************************
*                                                                               *
*                    P e r f o r m a n c e   C o u n t e r                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 2009,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXAtomic.h"
#include "FXRunnable.h"
#include "FXAutoThreadStorageKey.h"
#include "FXThread.h"
#include "FXPerformance.h"

/*
  Notes:
  - Counters for performance logging.

  - Usage:

        1. Declare a global variable of type Counter, (if PERFORMANCE_LOGGING enabled).
        2. In statement block, declare PERFORMANCE_COUNTER(blockcounter) to
           record time (in processor ticks) of the execution of the statement block.
        3. At the end, the global destructors of each Counter will dump out performance
           metrics.

  - Metrics recorded:

        1. Minimum, maximum time to execute statement block (processor ticks).
        2. Number of times statement block was executed.
        3. Total time spent in statement block (processor ticks).
        4. Average time spent in statement block (processor ticks).

  - Sources for inaccuracies: each processor core has its own tick counter, and while
    operating systems try to keep counters synchronized, there are no guarantees they
    are exactly in sync.  Thus, long executions which may experience context switches
    during measurements may be slightly inaccurate.
*/

/*******************************************************************************/

using namespace FX;

namespace FX {


// Before starting initialize counter data
FXCounter::FXCounter(const FXchar *const nm):name(nm),minticks(FXLONG(9223372036854775807)),maxticks(0),totticks(0),counter(0){
  }


// Tally results of measurement
void FXCounter::tally(FXlong ticks){
  atomicMin(&minticks,ticks);
  atomicMax(&maxticks,ticks);
  atomicAdd(&totticks,ticks);
  atomicAdd(&counter,1);
  }


// Upon exit from global scope, dump statistics
FXCounter::~FXCounter(){
  if(0<counter){
    FXlong avgticks=(totticks+(counter>>1))/counter;
    fxmessage("%-30.30s: avg:%'16lld min:%'16lld max:%'16lld tot:%'16lld cnt:%'12lld\n",name,avgticks,minticks,maxticks,totticks,counter);
    }
  }

}
