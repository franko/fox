/********************************************************************************
*                                                                               *
*                          E X E   I m a g e   O b j e c t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 2014,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "FXEXEImage.h"



/*
  Notes:
*/

using namespace FX;

/*******************************************************************************/

namespace FX {


// Suggested file extension
const FXchar FXEXEImage::fileExt[]="exe";


// Suggested mime type
const FXchar FXEXEImage::mimeType[]="application/octet-stream";


// Object implementation
FXIMPLEMENT(FXEXEImage,FXImage,nullptr,0)


// Initialize
FXEXEImage::FXEXEImage(FXApp* a,const FXuchar *pix,FXuint opts,FXint w,FXint h,FXint ri,FXint rt):FXImage(a,nullptr,opts,w,h),rtype(rt),rid(ri){
  if(pix){
    FXMemoryStream ms(FXStreamLoad,const_cast<FXuchar*>(pix));
    loadPixels(ms);
    }
  }


// Can not save pixels
FXbool FXEXEImage::savePixels(FXStream&) const {
  return false;
  }


// Load pixel data only
FXbool FXEXEImage::loadPixels(FXStream& store){
  FXColor *pixels; FXint w,h;
  if(fxloadEXE(store,pixels,w,h,rtype,rid)){
    setData(pixels,IMAGE_OWNED,w,h);
    return true;
    }
  return false;
  }


// Clean up
FXEXEImage::~FXEXEImage(){
  }

}
