/********************************************************************************
*                                                                               *
*                          Q O I F   I m a g e   O b j e c t                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2024 by Jeroen van der Zijp.   All Rights Reserved.             *
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
#ifndef FXQOIFIMAGE_H
#define FXQOIFIMAGE_H

#ifndef FXIMAGE_H
#include "FXImage.h"
#endif

namespace FX {


/// QOIF Image class
class FXAPI FXQOIFImage : public FXImage {
  FXDECLARE(FXQOIFImage)
protected:
  FXQOIFImage(){}
private:
  FXQOIFImage(const FXQOIFImage&);
  FXQOIFImage &operator=(const FXQOIFImage&);
public:
  static const FXchar fileExt[];
  static const FXchar mimeType[];
public:

  /// Construct an image from memory stream formatted in QOIF format
  FXQOIFImage(FXApp *a,const FXuchar *pix=nullptr,FXuint opts=0,FXint w=1,FXint h=1);

  /// Load pixels from stream in PNG format
  virtual FXbool savePixels(FXStream& store) const;

  /// Save pixels into stream in PNG format
  virtual FXbool loadPixels(FXStream& store);

  /// Destroy
  virtual ~FXQOIFImage();
  };


#ifndef FXLOADQOIF
#define FXLOADQOIF

/**
* Check if stream contains a QOIF, return true if so.
*/
extern FXAPI FXbool fxcheckQOIF(FXStream& store);


/**
* Load an QOIF (Quite OK Image Format) file from a stream.
* Upon successful return, the pixel array and size are returned.
* If an error occurred, the pixel array is set to NULL.
*/
extern FXAPI FXbool fxloadQOIF(FXStream& store,FXColor*& data,FXint& width,FXint& height);


/**
* Save an QOIF (Quite OK Image Format) file to a stream.
*/
extern FXAPI FXbool fxsaveQOIF(FXStream& store,const FXColor* data,FXint width,FXint height);

#endif

}

#endif
