/********************************************************************************
*                                                                               *
*                      I R I S   R G B   I c o n   O b j e c t                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "FXArray.h"
#include "FXMetaClass.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXMemoryStream.h"
#include "FXRGBIcon.h"


/*
  Notes:
*/

using namespace FX;

/*******************************************************************************/

namespace FX {


// Suggested file extension
const FXchar FXRGBIcon::fileExt[]="rgb";


// Suggested mime type
const FXchar FXRGBIcon::mimeType[]="image/rgb";


// Object implementation
FXIMPLEMENT(FXRGBIcon,FXIcon,nullptr,0)


// Initialize nicely
FXRGBIcon::FXRGBIcon(FXApp* a,const FXuchar *pix,FXColor clr,FXuint opts,FXint w,FXint h):FXIcon(a,nullptr,clr,opts,w,h){
  if(pix){
    FXMemoryStream ms(FXStreamLoad,const_cast<FXuchar*>(pix));
    loadPixels(ms);
    }
  }


// Save object to stream
FXbool FXRGBIcon::savePixels(FXStream& store) const {
  if(fxsaveRGB(store,data,width,height)){
    return true;
    }
  return false;
  }


// Load object from stream
FXbool FXRGBIcon::loadPixels(FXStream& store){
  FXColor *pixels; FXint w,h;
  if(fxloadRGB(store,pixels,w,h)){
    setData(pixels,IMAGE_OWNED,w,h);
    if(options&IMAGE_ALPHAGUESS) setTransparentColor(guesstransp());
    if(options&IMAGE_THRESGUESS) setThresholdValue(guessthresh());
    return true;
    }
  return false;
  }


// Clean up
FXRGBIcon::~FXRGBIcon(){
  }

}
