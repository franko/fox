/********************************************************************************
*                                                                               *
*                          P N G   I m a g e   O b j e c t                      *
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
#include "FXElement.h"
#include "FXMetaClass.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXMemoryStream.h"
#include "FXPNGImage.h"
#include "FXPNGIcon.h"


/*
  Notes:
  - FXPNGIcon has an alpha channel
*/

using namespace FX;

/*******************************************************************************/

namespace FX {


// Suggested file extension
const FXchar FXPNGIcon::fileExt[]="png";


// Suggested mime type
const FXchar FXPNGIcon::mimeType[]="image/png";


// Object implementation
FXIMPLEMENT(FXPNGIcon,FXIcon,nullptr,0)


#ifdef HAVE_ZLIB_H
const FXbool FXPNGIcon::supported=true;
#else
const FXbool FXPNGIcon::supported=false;
#endif


// Initialize
FXPNGIcon::FXPNGIcon(FXApp* a,const FXuchar *pix,FXColor clr,FXuint opts,FXint w,FXint h,FXuint fl):FXIcon(a,nullptr,clr,opts,w,h),flags(fl){
  if(pix){
    FXMemoryStream ms(FXStreamLoad,const_cast<FXuchar*>(pix));
    loadPixels(ms);
    }
  }


// Save pixels only
FXbool FXPNGIcon::savePixels(FXStream& store) const {
  if(fxsavePNG(store,data,width,height,flags)){
    return true;
    }
  return false;
  }


// Load pixels only
FXbool FXPNGIcon::loadPixels(FXStream& store){
  FXColor *pixels; FXint w,h;
  if(fxloadPNG(store,pixels,w,h)){
    setData(pixels,IMAGE_OWNED,w,h);
    if(options&IMAGE_ALPHAGUESS) setTransparentColor(guesstransp());
    if(options&IMAGE_THRESGUESS) setThresholdValue(guessthresh());
    return true;
    }
  return false;
  }


// Clean up
FXPNGIcon::~FXPNGIcon(){
  }

}
