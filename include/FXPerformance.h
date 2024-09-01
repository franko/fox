/********************************************************************************
*                                                                               *
*                    P e r f o r m a n c e   C o u n t e r                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 2009,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
********************************************************************************/
#ifndef FXPERFORMANCE_H
#define FXPERFORMANCE_H

namespace FX {


// Performance measurement counter
class FXAPI FXCounter {
private:
  const FXchar *const name;
  volatile FXlong     minticks;
  volatile FXlong     maxticks;
  volatile FXlong     totticks;
  volatile FXlong     counter;
public:
  FXCounter(const FXchar *const nm);
  void tally(FXlong ticks);
 ~FXCounter();
  };


// Measure performance of code in scope
template<FXCounter& counter>
class FXPerformanceCounter {
private:
  volatile FXlong ticks;
public:
  FXPerformanceCounter(){ticks=FXThread::ticks();}
 ~FXPerformanceCounter(){counter.tally(FXThread::ticks()-ticks);}
  };



// Invoke counter if enabled
#if defined(PERFORMANCE_LOGGING)
#define PERFORMANCE_RECORDER(counter)    FXCounter perf##counter(#counter)
#define PERFORMANCE_COUNTER(counter)     FXPerformanceCounter< perf##counter > measure##counter
#else
#define PERFORMANCE_RECORDER(counter)
#define PERFORMANCE_COUNTER(counter)
#endif

}

#endif
