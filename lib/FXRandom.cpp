/********************************************************************************
*                                                                               *
*                  R a n d o m   N u m b e r   G e n e r a t o r                *
*                                                                               *
*********************************************************************************
* Copyright (C) 2007,2022 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxmath.h"
#include "FXRandom.h"


/*
  Notes:
  - Algorithm based on Numerical Recipes, 3ed, pp. 351-352.

  - Original algorithm George Marsaglia, "Random number generators",
    Journal of Modern Applied Statistical Methods 2, No. 2, 2003.

  - Different shift-counts are certainly possible, and produce different
    sequences (of the same period), see G. Marsaglia, "Xorshift RNGs".

  - New shift counts based on the paper "An experimental exploration of
    Marsaglia's xorshift generators, scrambled", Sebastiano Vigna, 2014.

  - We note that the x86-64 processors don't have unsigned long -> float and
    unsigned long -> double conversions, causing compilers to emit a branch
    based on the sign.

    Thus, we interpret the unsigned long as signed long, perform a long->float
    or long->double conversion, and then take the absolute value.

    This has the effect of making the algorithm branch-free.

    Even through small, the effect of this little change is HUGE in terms of
    performance, due to mis-prediction of branches dependent on random inputs.

*/

#define SHIFTA    12
#define SHIFTB    25
#define SHIFTC    27

using namespace FX;


namespace FX {

/*******************************************************************************/

// Construct random generator with default seed
FXRandom::FXRandom():state(FXULONG(4101842887655102017)){
  }


// Construct random generator with given seed s
FXRandom::FXRandom(FXulong s):state(FXULONG(4101842887655102017)^s){
  }


// Initialize seed
void FXRandom::seed(FXulong s){
  state=s^FXULONG(4101842887655102017);
  }


// Generate next state
FXulong FXRandom::next(){
  state^=state>>SHIFTA;
  state^=state<<SHIFTB;
  state^=state>>SHIFTC;
  return state;
  }


// Get random long
FXulong FXRandom::randLong(){
  return next()*FXULONG(2685821657736338717);
  }


// Get random float
// We're drawing an unsigned-long, but interpreting this as a signed-long;
// this enables a signed-integer to float conversion, which is a single
// instruction (cvtsi2ssq) on Intel CPU.
// Then just scale and drop the sign; resulting code is branch-free and
// almost twice as fast as simply converting an unsigned-long to float;
// this is because with random-inputs, branch is mispredicted very often!
FXfloat FXRandom::randFloat(){
  FXlong num=(FXlong)randLong();
  return Math::fabs(num*1.0842021724855044340074528008699e-19f);
  }


// Get random double
// We're drawing an unsigned-long, but interpreting this as a signed-long;
// this enables a signed-integer to double conversion, which is a single
// instruction (cvtsi2sdq) on Intel CPU.
// Then just scale and drop the sign; resulting code is branch-free and
// almost twice as fast as simply converting an unsigned-long to double;
// this is because with random-inputs, branch is mispredicted very often!
FXdouble FXRandom::randDouble(){
  FXlong num=(FXlong)randLong();
  return Math::fabs(num*1.0842021724855044340074528008699e-19);
  }

}



