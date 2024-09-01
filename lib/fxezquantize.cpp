/********************************************************************************
*                                                                               *
*                   E Z   C o l o r   Q u a n t i z a t i o n                   *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "fxendian.h"
#include "FXElement.h"


/*
  Notes:

  - Build 8-bit colormapped image from full 24 (plus alpha) source image.

  - Algorithm: for each pixel in source image, check hash table of colors to
    see if this color has been encountered already.

    If it is a new color, and there are still unused slots in the colormap,
    assign the color a new index and add it to the hash table.

    If color already has been assigned a colormap index, write this index
    to the output image.

  - Process will fail if the source image has too many colors to fit the
    available colormap maximum (up to 256).

  - If successful, the entire image can be losslessly represented as an
    colormapped image, and the colormap and corresponding image will be
    returned.

  - Updated version assigned indexes to colors in one pass, as it hashes
    colors into table.  However, if too many colors are seen as the image
    is being processed, not all output pixels will have been assigned.

  - FIXME this may be added as an option: pixels with 0 alpha, i.e. fully
    transparant pixels, can be optionally mapped to a single colormap entry,
    by squashing such pixels to one particular value (0 in the current
    version).
*/


using namespace FX;


/*******************************************************************************/

namespace FX {


extern FXbool fxezquantize(FXuchar* dst,const FXColor* src,FXColor* colormap,FXint& actualcolors,FXint w,FXint h,FXint maxcolors);


// Simple but quite fast "hash" function
static inline FXuint HashColor(FXColor clr){
  return clr*0x9E3779B1;
  }


// EZ quantization may be used if w*h<=maxcolors, or if the actual colors
// used is less than maxcolors; using fxezquantize assures that no
// loss of data occurs repeatedly loading and saving the same file!
FXbool fxezquantize(FXuchar* dst,const FXColor* src,FXColor* colormap,FXint& actualcolors,FXint w,FXint h,FXint maxcolors){
  const FXColor ALPHA=FXRGBA(0,0,0,255);        // Mask alpha channel
  if(maxcolors<=256){
    FXColor colortable[512];                    // Colors encountered in image
    FXshort indextable[512];                    // Map index assigned to color
    FXuint  npixels=w*h;
    FXuint  i,p,b,x;
    FXint   ncolors=0;
    FXColor color;

    // Clear map index
    fillElms(indextable,-1,ARRAYNUMBER(indextable));

    // Hash all colors from image
    for(i=0; i<npixels; i++){

      // Get pixel
      color=src[i];

#if 0
      // Squash fully transparant colors to
      // special alpha color value
      if((color&ALPHA)==0) color=0;
#endif

      // Find color's position in table
      p=b=HashColor(color);
      while(__likely(0<=indextable[x=p&511])){
        if(__likely(colortable[x]==color)) goto ass;
        p=(p<<2)+p+b+1;
        b>>=5;
        }

      // Not fitting
      if(__unlikely(ncolors>=maxcolors)) goto x;

      // Map color to index
      FXASSERT(x<512);
      colortable[x]=color;
      indextable[x]=ncolors;

      // New color
      FXASSERT(ncolors<maxcolors);
      colormap[ncolors++]=color;

      // Assign output pixel
ass:  dst[i]=(FXuchar)indextable[x];
      }

    // Actual number of colors used
    FXASSERT(ncolors<=maxcolors);
    actualcolors=ncolors;

    return true;
    }
x:return false;
  }

}
