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
#include "FXRunnable.h"
#include "FXAutoThreadStorageKey.h"
#include "FXThread.h"
#include "FXPNGImage.h"



/*
  Notes:
  - FXPNGImage has an alpha channel.
*/

using namespace FX;

/*******************************************************************************/

namespace FX {


// Suggested file extension
const FXchar FXPNGImage::fileExt[]="png";


// Suggested mime type
const FXchar FXPNGImage::mimeType[]="image/png";


// Object implementation
FXIMPLEMENT(FXPNGImage,FXImage,nullptr,0)


#ifdef HAVE_ZLIB_H
const FXbool FXPNGImage::supported=true;
#else
const FXbool FXPNGImage::supported=false;
#endif


// Initialize
FXPNGImage::FXPNGImage(FXApp* a,const FXuchar *pix,FXuint opts,FXint w,FXint h,FXuint fl):FXImage(a,nullptr,opts,w,h),flags(fl){
  if(pix){
    FXMemoryStream ms(FXStreamLoad,const_cast<FXuchar*>(pix));
    loadPixels(ms);
    }
  }


// Save the pixels only
FXbool FXPNGImage::savePixels(FXStream& store) const {
  if(fxsavePNG(store,data,width,height,flags)){
    return true;
    }
  return false;
  }


// Load pixels only
FXbool FXPNGImage::loadPixels(FXStream& store){
  FXColor *pixels; FXint w,h;
  if(fxloadPNG(store,pixels,w,h)){
    setData(pixels,IMAGE_OWNED,w,h);
    return true;
    }
  return false;
  }


// Clean up
FXPNGImage::~FXPNGImage(){
  }

}
