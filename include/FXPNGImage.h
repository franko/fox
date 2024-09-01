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
#ifndef FXPNGIMAGE_H
#define FXPNGIMAGE_H

#ifndef FXIMAGE_H
#include "FXImage.h"
#endif

namespace FX {


/**
* PNG Image save-flags.
*/
enum {
  PNG_FILTER_NONE   = 0,        // No filter
  PNG_FILTER_SUB    = 1,        // Sub-filter
  PNG_FILTER_UP     = 2,        // Up-filter
  PNG_FILTER_AVG    = 3,        // Averaging filter
  PNG_FILTER_PAETH  = 4,        // Paeth filter
  PNG_FILTER_BEST   = 5,        // Find best filter for each line
  PNG_COMPRESS_FAST = 8,        // Fastest compression
  PNG_COMPRESS_BEST = 16,       // Best compression
  PNG_IMAGE_GRAY    = 32,       // Write only one grey channel (blue)
  PNG_IMAGE_OPAQUE  = 64,       // Write no alpha alpha channel
  PNG_IMAGE_ANALYZE = 128,      // Analyze image for opacity or alpha
  PNG_INDEX_COLOR   = 256       // Try indexed (colormap) mode
  };



/// Portable Network Graphics (PNG) Image class
class FXAPI FXPNGImage : public FXImage {
  FXDECLARE(FXPNGImage)
protected:
  FXuint flags;
protected:
  FXPNGImage(){}
private:
  FXPNGImage(const FXPNGImage&);
  FXPNGImage &operator=(const FXPNGImage&);
public:
  static const FXchar fileExt[];
  static const FXchar mimeType[];
public:

  /// Construct an image from memory stream formatted in PNG format
  FXPNGImage(FXApp *a,const FXuchar *pix=nullptr,FXuint opts=0,FXint w=1,FXint h=1,FXuint fl=PNG_IMAGE_ANALYZE);

  /// True if format is supported
  static const FXbool supported;

  /// Set image save flags
  void setFlags(FXint opts){ options=opts; }

  /// Get image save flags
  FXint getFlags() const { return options; }

  /// Load pixels from stream in PNG format
  virtual FXbool savePixels(FXStream& store) const;

  /// Save pixels into stream in PNG format
  virtual FXbool loadPixels(FXStream& store);

  /// Destroy
  virtual ~FXPNGImage();
  };



/**
* Check if stream contains a PNG, return true if so.
*/
extern FXAPI FXbool fxcheckPNG(FXStream& store);


/**
* Load an PNG (Portable Network Graphics) file from a stream.
* Upon successful return, the pixel array and size are returned.
* If an error occurred, the pixel array is set to NULL.
*/
extern FXAPI FXbool fxloadPNG(FXStream& store,FXColor*& data,FXint& width,FXint& height);


/**
* Save an PNG (Portable Network Graphics) file to a stream.
*/
extern FXAPI FXbool fxsavePNG(FXStream& store,const FXColor* data,FXint width,FXint height,FXuint flags=PNG_IMAGE_ANALYZE);

}

#endif
