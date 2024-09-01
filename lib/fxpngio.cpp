/********************************************************************************
*                                                                               *
*                         P N G    I n p u t / O u t p u t                      *
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
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxcrc.h"
#include "fxmath.h"
#include "fxendian.h"
#include "FXElement.h"
#include "FXMetaClass.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXRunnable.h"
#include "FXAutoThreadStorageKey.h"
#include "FXThread.h"
#include "FXPerformance.h"
#include "FXPNGImage.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif


/*
  Notes:

  - PNG Image I/O now only requires presence of the Zlib compression
    library.  The libpng image I/O support library is no longer necessary.

  - When saving PNG image, flags control how this is done:

      PNG_FILTER_NONE   Do not use entropy reduction filter; this is
                        default for images with <8 bits/pixel.

      PNG_FILTER_BEST   For each line, figure which filter is best.

      PNG_FILTER_SUB    Use horizontal prediction filter.
      PNG_FILTER_UP     Use vertical prediction filter.
      PNG_FILTER_AVG    Use averaging prediction filter.
      PNG_FILTER_PAETH  Use Paeth diagonal prediction filter.

    Compression is selected by:

      PNG_COMPRESS_FAST Fastest compression mode, but less compression
                        will be possible.

      PNG_COMPRESS_BEST Highest compression, at the expense of slower
                        execution.

    Reduction in channels:

      PNG_IMAGE_GRAY    Assume image is gray-level; assuming R=G=B,
                        only the blue channel needs to be saved.

      PNG_IMAGE_OPAQUE  Assume image has no transparency, so no
                        alpha-channel needs to be saved.  Can be used
                        in combination with PNG_IMAGE_GRAY.

      PNG_IMAGE_ANALYZE Scan the image to see whether fewer channels
                        can be saved; if this fails then all R,G,B, and
                        A will be saved.

    Writing Indexed [colormapped] image:

      PNG_INDEX_COLOR   Check to see if image has fewer than 256 colors;
                        if so, we can drop to 8-bit per pixel, or even
                        lower, to 4-, 2-, or 1-bit per pixel, depending
                        on how many colors are in image.

  - We handle ADAM7 interlaced PNG images, but will only return fully decoded
    images.  Adam7 interlace pattern:

      1 6 4 6 2 6 4 6 . . .
      7 7 7 7 7 7 7 7 . . .
      5 6 5 6 5 6 5 6 . . .
      7 7 7 7 7 7 7 7 . . .
      3 6 4 6 3 6 4 6 . . .
      7 7 7 7 7 7 7 7 . . .
      5 6 5 6 5 6 5 6 . . .
      7 7 7 7 7 7 7 7 . . .
      . . . . . . . . . . .
      . . . . . . . . . . .

  - Prior to feeding through Zlib compression library, entropy-reduction
    filter can detect horizontal or vertical gradients, etc., and yield
    markedly better compression ratios.  This make take slightly more
    time but could be worthwhile for smallest outputs.

    Given 2x2 pixel environment around pixel of interest x:

      . c b . . .
      . a x . . .

    Encoding/Filtering:

      0  None   Filt(x) = Orig(x)
      1  Sub    Filt(x) = Orig(x) - Orig(a)
      2  Up     Filt(x) = Orig(x) - Orig(b)
      3  Avg    Filt(x) = Orig(x) - floor((Orig(a)+Orig(b))/2
      4  Paeth  Filt(x) = Orig(x) - PaethPredictor(Orig(a),Orig(b),Orig(c))

    Decoding/Reconstruction:

      0  None   Recon(x) = Filt(x)
      1  Sub    Recon(x) = Filt(x) + Recon(a)
      2  Up     Recon(x) = Filt(x) + Recon(b)
      3  Avg    Recon(x) = Filt(x) + floor((Recon(a)+Recon(b)) /2
      4  Paeth  Recon(x) = Filt(x) + PaethPredictor(Recon(a),Recon(b),Recon(c))

  - Technically, we should check special "alphacolor" by 16-bit/channel
    comparisons, not post-conversion FXColor to FXColor.  But not sure
    how much of a problem it really is, once you use 16-bit/channel
    depth you're already dealing with large images, anyway...

  - Possible future improvements:

    1) Detect on-off alpha case, i.e. pixels either opaque or fully
       transparent.  This allows alpha to be encoded using unused
       color [if there is one], and thus support alpha even though
       not saving explicit alpha channel.

    2) When detecting gray-level image, we should check how many levels
       of gray are actually available, and if they're uniformly distributed
       through the gray range.  This could result in 1-bpp, 2-bpp, or 4-bpp
       image with no colormap.

    3) Check into faster compression library. ZLib is quite slow, but we
       can only select another compression method if this leads to a compatible
       bit-stream to the one produced by Zlib.

    4) Perhaps option to write interlaced image would be useful.  However,
       FOX currently does not take advantage of this so for now this is
       only of marginal interest.

    5) Scaling 16-bits/channel images is not *entirely* correct; one may
       use x/257 = ((x*2139127681)>>32)>>7 as a way to improve the situation,
       but it'll be loads slower and one may not see the difference.

  - References:

    http://www.w3.org/TR/REC-png.html
    http://www.graphicswiz.com/png/
    http://www.inforamp.net/~poynton
    http://www.libpng.org/pub/png/
    https://libspng.org/download.html (new!)
*/

#define TOPIC_DETAIL 1015

// Turn chunk characters into a single number
#define CHUNK(c0,c1,c2,c3) ((c3)|((c2)<<8)|((c1)<<16)|((c0)<<24))

using namespace FX;

/*******************************************************************************/

namespace FX {

#ifdef HAVE_ZLIB_H  /////////////////////////////////////////////////////////////

union RGBAPixel {
  struct {
    FXuchar b;
    FXuchar g;
    FXuchar r;
    FXuchar a;
    };
  struct {
    FXColor c;
    };
  };

// PNG file signature
const FXuchar signature[8]={
  0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A
  };

// Interleaving x
const FXuint xoffset[] = {0,4,0,2,0,1,0};
const FXuint xstep[]   = {8,8,4,4,2,2,1};
const FXuint xshift[]  = {3,3,2,2,1,1,0};

// Interleaving y
const FXuint yoffset[] = {0,0,4,0,2,0,1};
const FXuint ystep[]   = {8,8,8,4,4,2,2};
const FXuint yshift[]  = {3,3,3,2,2,1,1};

// 1-bit map black/white
const FXColor map1Bit[2]={
  FXRGBA(  0,  0,  0,255),
  FXRGBA(255,255,255,255)
  };

// 2-bit map grey
const FXColor map2Bit[4]={
  FXRGBA(  0,  0,  0,255),
  FXRGBA( 85, 85, 85,255),
  FXRGBA(170,170,170,255),
  FXRGBA(255,255,255,255)
  };

// 4-bit map grey
const FXColor map4Bit[16]={
  FXRGBA(  0,  0,  0,255),
  FXRGBA( 17, 17, 17,255),
  FXRGBA( 34, 34, 34,255),
  FXRGBA( 51, 51, 51,255),
  FXRGBA( 68, 68, 68,255),
  FXRGBA( 85, 85, 85,255),
  FXRGBA(102,102,102,255),
  FXRGBA(119,119,119,255),
  FXRGBA(136,136,136,255),
  FXRGBA(153,153,153,255),
  FXRGBA(170,170,170,255),
  FXRGBA(187,187,187,255),
  FXRGBA(204,204,204,255),
  FXRGBA(221,221,221,255),
  FXRGBA(238,238,238,255),
  FXRGBA(255,255,255,255)
  };

// Filters
const FXuchar  FiltNone  = 0;
const FXuchar  FiltSub   = 1;
const FXuchar  FiltUp    = 2;
const FXuchar  FiltAvg   = 3;
const FXuchar  FiltPaeth = 4;

// Compression
const FXuchar Deflate  = 0;

// Interlacing
const FXuchar NoInterlace = 0;
const FXuchar Adam7       = 1;

// Image type
const FXuchar Gray      = 0;
const FXuchar RGB       = 2;
const FXuchar Indexed   = 3;
const FXuchar GrayAlpha = 4;
const FXuchar RGBA      = 6;

// Number of channels for image type
const FXuint channels[8]={
  1,0,3,1,2,0,4,0
  };

// Base 2 log of bits/channel (1,2,4,8,16)
const FXuchar logBitdepth[17]={
  0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4
  };

// Sets of allowable bitdepths for image type
const FXuint depths[8]={
  0x10116,0,0x10100,0x00116,0x10100,0,0x10100,0
  };

// Chunk ids
const FXuint IHDR=CHUNK('I','H','D','R');
const FXuint tIME=CHUNK('t','I','M','E');
const FXuint zTXt=CHUNK('z','T','X','t');
const FXuint tEXt=CHUNK('t','E','X','t');
const FXuint iTXt=CHUNK('i','T','X','t');
const FXuint pHYs=CHUNK('p','H','Y','s');
const FXuint sPLT=CHUNK('s','P','L','T');
const FXuint eXIf=CHUNK('e','X','I','f');
const FXuint acTL=CHUNK('a','c','T','L');
const FXuint iCCP=CHUNK('i','C','C','P');
const FXuint cICP=CHUNK('c','I','C','P');
const FXuint sRGB=CHUNK('s','R','G','B');
const FXuint sBIT=CHUNK('s','B','I','T');
const FXuint gAMA=CHUNK('g','A','M','A');
const FXuint cHRM=CHUNK('c','H','R','M');
const FXuint mDCv=CHUNK('m','D','C','v');
const FXuint cLLi=CHUNK('c','L','L','i');
const FXuint PLTE=CHUNK('P','L','T','E');
const FXuint tRNS=CHUNK('t','R','N','S');
const FXuint hIST=CHUNK('h','I','S','T');
const FXuint bKGD=CHUNK('b','K','G','D');
const FXuint IDAT=CHUNK('I','D','A','T');
const FXuint fdAT=CHUNK('f','d','A','T');
const FXuint fcTL=CHUNK('f','c','T','L');
const FXuint IEND=CHUNK('I','E','N','D');

// Filter designation
const FXuint PNG_FILTER_MASK=(PNG_FILTER_SUB|PNG_FILTER_UP|PNG_FILTER_AVG|PNG_FILTER_BEST);

/*******************************************************************************/

// PNG Decoder
class PNGDecoder {
public:
  FXColor      *image;                  // Output Image
  FXuchar      *buffer;                 // Zlib input/output buffer
  FXuint        buffersize;             // Zlib output buffer size
  z_stream      stream;                 // Zlib stream
  FXuint        width;                  // Width
  FXuint        height;                 // Height
  FXuchar       imagetype;              // Image type (Gray, Indexed, ...)
  FXuchar       bitdepth;               // Bit depth per channel
  FXuchar       compression;            // Compression mode
  FXuchar       filter;                 // Filter
  FXuchar       interlace;              // Interlace (NoInterlace, Adam7)
  FXuint        stride;                 // Pixel stride
  FXuint        numbytes;               // Number of bytes/line
  FXuint        totbytes;               // Number of total bytes
  FXuint        ncolormap;              // Number of colors in colormap
  RGBAPixel     colormap[256];          // Colormap
  FXuint        intwidth[7];            // Widgth of image for each interlace pass
  FXuint        intheight[7];           // Height of image for each interlace pass
  FXuint        intbytes[7];            // Number of bytes for each interlace pass
  FXushort      backcolor[3];           // Background color spec
  FXushort      alfacolor[3];           // Alpha color spec
public:
  FXbool header(FXStream& store);
  FXbool palette(FXStream& store,FXuint length);
  FXbool background(FXStream& store,FXuint length);
  FXbool transparency(FXStream& store,FXuint length);
  void applyTransparency();
  FXbool decode();
  FXbool data(FXStream& store,FXuint length);
  FXbool end(FXStream& store,FXuint length);
public:

  // Initialize decoder
  PNGDecoder():image(nullptr),buffer(nullptr),buffersize(0),width(0),height(0),imagetype(Indexed),bitdepth(8),compression(Deflate),filter(FiltNone),interlace(NoInterlace),stride(0),numbytes(0),totbytes(0),ncolormap(0){
    backcolor[0]=backcolor[1]=backcolor[2]=0;
    alfacolor[0]=alfacolor[1]=alfacolor[2]=0;
    clearElms(&stream,1);
    }

  // Load image
  FXbool load(FXStream& store,FXColor*& data,FXint& w,FXint& h);
  };


// Load and validate header
FXbool PNGDecoder::header(FXStream& store){
  FXuint crc=CRC32::CRC(~0,IHDR);
  FXuint chunklength;
  FXuint chunkid;
  FXuint chunkcrc;
  FXuint ch;

  // Chunk length
  store >> chunklength;
  store >> chunkid;

  // Bad size or id
  if(chunklength!=13) return false;
  if(chunkid!=IHDR) return false;

  // Grab size
  store >> width;
  store >> height;

  // Bad size
  if(width==0) return false;
  if(height==0) return false;
  if(width>2147483647/height) return false;

  // Grab bit depth
  store >> bitdepth;

  // Grab image type
  store >> imagetype;

  // Bad image type
  if(imagetype>RGBA) return false;

  // Bad bit depth
  if(FXBIT(depths[imagetype],bitdepth)==0) return false;

  // Grab compression
  store >> compression;

  // Bad compression
  if(compression!=Deflate) return false;

  // Grab filter
  store >> filter;

  // Bad filter
  if(filter!=0) return false;

  // Get interlace
  store >> interlace;

  // Bad interlace
  if(interlace>Adam7) return false;

  // Calculate CRC
  crc=CRC32::CRC(crc,width);
  crc=CRC32::CRC(crc,height);
  crc=CRC32::CRC(crc,bitdepth);
  crc=CRC32::CRC(crc,imagetype);
  crc=CRC32::CRC(crc,compression);
  crc=CRC32::CRC(crc,filter);
  crc=CRC32::CRC(crc,interlace);

  // Checksum
  store >> chunkcrc;
  if(chunkcrc!=~crc){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: IHDR crc mismatch\n"));
    return false;
    }

  // Compute some handy values
  ch=channels[imagetype];
  numbytes=(width*ch*bitdepth+7)>>3;
  totbytes=numbytes*height+height;      // One extra byte/line
  stride=(ch*bitdepth+7)>>3;

  FXTRACE((TOPIC_DETAIL,"fxloadPNG: Header:\n"));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: width     = %u\n",width));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: height    = %u\n",height));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: imagetype = %s\n",imagetype==Gray?"Gray":imagetype==RGB?"RGB":imagetype==Indexed?"Indexed":imagetype==GrayAlpha?"GrayAlpha":imagetype==RGBA?"RGBA":"Invalid"));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: channels  = %u\n",ch));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: bitdepth  = %u\n",bitdepth));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: compress  = %u\n",compression));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: filter    = %u\n",filter));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: interlace = %s\n",interlace?"Adam7":"None"));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: stride    = %u\n",stride));
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: numbytes  = %u\n",numbytes));

  // Set up interlacing
  if(interlace==Adam7){
    totbytes=0;
    for(FXuint p=0; p<7; ++p){
      if(xoffset[p]<width && yoffset[p]<height){
        intbytes[p]=(((width-xoffset[p]+xstep[p]-1)>>xshift[p])*bitdepth*ch+7)>>3;
        intwidth[p]=(width-xoffset[p]+xstep[p]-1)>>xshift[p];
        intheight[p]=(height-yoffset[p]+ystep[p]-1)>>yshift[p];
        totbytes+=intbytes[p]*intheight[p]+intheight[p];        // One extra byte/line
        }
      else{
        intbytes[p]=0;
        intwidth[p]=0;
        intheight[p]=0;
        }
      }
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: intbytes  = %5u %5u %5u %5u %5u %5u %5u\n",intbytes[0],intbytes[1],intbytes[2],intbytes[3],intbytes[4],intbytes[5],intbytes[6]));
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: intwidth  = %5u %5u %5u %5u %5u %5u %5u\n",intwidth[0],intwidth[1],intwidth[2],intwidth[3],intwidth[4],intwidth[5],intwidth[6]));
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: intheight = %5u %5u %5u %5u %5u %5u %5u\n",intheight[0],intheight[1],intheight[2],intheight[3],intheight[4],intheight[5],intheight[6]));
    }

  FXTRACE((TOPIC_DETAIL,"fxloadPNG: totbytes  = %u\n",totbytes));

  // OK?
  return (store.status()==FXStreamOK);
  }


PERFORMANCE_RECORDER(PNGDecoder_palette);


// Load palette
FXbool PNGDecoder::palette(FXStream& store,FXuint length){
  PERFORMANCE_COUNTER(PNGDecoder_palette);
  FXuint crc=CRC32::CRC(~0,PLTE);
  FXuint chunkcrc=0;
  FXuchar r,g,b;

  FXTRACE((TOPIC_DETAIL,"fxloadPNG: PLTE length = %u\n",length));

  // Number of colors
  ncolormap=length/3;

  // Too many of colors
  if(__unlikely(256<ncolormap)){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: PLTE: unexpected length = %u\n",length));
    return false;
    }

  // Grab colormap
  for(FXuint c=0; c<ncolormap; ++c){
    store >> r;
    store >> g;
    store >> b;
    crc=CRC32::CRC(crc,r);
    crc=CRC32::CRC(crc,g);
    crc=CRC32::CRC(crc,b);
    colormap[c].r=r;
    colormap[c].g=g;
    colormap[c].b=b;
    colormap[c].a=255;
    }

  FXTRACE((TOPIC_DETAIL,"fxloadPNG: ncolormap = %u\n",ncolormap));

  // Checksum
  store >> chunkcrc;
  if(chunkcrc!=~crc){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: PLTE crc mismatch\n"));
    return false;
    }
  return (store.status()==FXStreamOK);
  }


// Load background
FXbool PNGDecoder::background(FXStream& store,FXuint length){
  FXuint   crc=CRC32::CRC(~0,bKGD);
  FXuint   chunkcrc=0;
  FXushort r,g,b;
  FXuchar  x;

  FXTRACE((TOPIC_DETAIL,"fxloadPNG: bKGD length = %u\n",length));

  // Grab backcolor
  switch(imagetype){
  case Indexed:
    if(__unlikely(length!=1)){
      FXTRACE((TOPIC_DETAIL,"fxloadPNG: bKGD: unexpected length = %u\n",length));
      return false;
      }
    store >> x;
    crc=CRC32::CRC(crc,x);
    backcolor[0]=colormap[x].r;
    backcolor[1]=colormap[x].g;
    backcolor[2]=colormap[x].b;
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: bKGD = (%3u %3u %3u)\n",backcolor[0],backcolor[1],backcolor[2]));
    break;
  case RGB:
  case RGBA:
    if(__unlikely(length!=6)){
      FXTRACE((TOPIC_DETAIL,"fxloadPNG: bKGD: unexpected length = %u\n",length));
      return false;
      }
    store >> r;
    store >> g;
    store >> b;
    crc=CRC32::CRC(crc,r);
    crc=CRC32::CRC(crc,g);
    crc=CRC32::CRC(crc,b);
    backcolor[0]=r;
    backcolor[1]=g;
    backcolor[2]=b;
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: bKGD = (%3u %3u %3u)\n",backcolor[0],backcolor[1],backcolor[2]));
    break;
  case Gray:
  case GrayAlpha:
    if(__unlikely(length!=2)){
      FXTRACE((TOPIC_DETAIL,"fxloadPNG: bKGD: unexpected length = %u\n",length));
      return false;
      }
    store >> g;
    crc=CRC32::CRC(crc,g);
    backcolor[0]=g;
    backcolor[1]=g;
    backcolor[2]=g;
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: bKGD = (%3u %3u %3u)\n",backcolor[0],backcolor[1],backcolor[2]));
    break;
  default:
    __unreachable();
    }

  // Checksum
  store >> chunkcrc;
  if(chunkcrc!=~crc){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: bKGD crc mismatch\n"));
    return false;
    }
  return (store.status()==FXStreamOK);
  }


PERFORMANCE_RECORDER(PNGDecoder_transparency);


// Load transparency information
// this is either an array of alpha-values to be applied to
// the colormap, or a special alpha-color to be set to fully
// transparent during the population of the image data.
FXbool PNGDecoder::transparency(FXStream& store,FXuint length){
  PERFORMANCE_COUNTER(PNGDecoder_transparency);
  FXuint   crc=CRC32::CRC(~0,tRNS);
  FXuint   chunkcrc=0;
  FXushort r,g,b;
  FXuchar  x;

  FXTRACE((TOPIC_DETAIL,"fxloadPNG: tRNS length = %u\n",length));

  // Grab transparency
  switch(imagetype){
  case Indexed:
    if(__unlikely(length>ncolormap)){
      FXTRACE((TOPIC_DETAIL,"fxloadPNG: tRNS: unexpected length = %u\n",length));
      return false;
      }
    for(FXuint c=0; c<length; ++c){
      store >> x;
      crc=CRC32::CRC(crc,x);
      colormap[c].a=x;
      }
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: tRNS: nalphas = %u\n",length));
    break;
  case RGB:
  case RGBA:
    if(__unlikely(length!=6)){
      FXTRACE((TOPIC_DETAIL,"fxloadPNG: tRNS: unexpected length = %u\n",length));
      return false;
      }
    store >> r;
    store >> g;
    store >> b;
    crc=CRC32::CRC(crc,r);
    crc=CRC32::CRC(crc,g);
    crc=CRC32::CRC(crc,b);
    alfacolor[0]=r;
    alfacolor[1]=g;
    alfacolor[2]=b;
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: tRNS = (%3u %3u %3u)\n",alfacolor[0],alfacolor[1],alfacolor[2]));
    break;
  case Gray:
  case GrayAlpha:
    if(__unlikely(length!=2)){
      FXTRACE((TOPIC_DETAIL,"fxloadPNG: tRNS: unexpected length = %u\n",length));
      return false;
      }
    store >> g;
    crc=CRC32::CRC(crc,g);
    alfacolor[0]=g;
    alfacolor[1]=g;
    alfacolor[2]=g;
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: tRNS = (%3u %3u %3u)\n",alfacolor[0],alfacolor[1],alfacolor[2]));
    break;
  default:
    __unreachable();
    }

  // Checksum
  store >> chunkcrc;
  if(chunkcrc!=~crc){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: tRNS crc mismatch\n"));
    return false;
    }
  return (store.status()==FXStreamOK);
  }


// Apply transparancy, i.e. a special color designated to be
// used as fully transparent.
// FIXME Technically, this should be made to work for
// bitdepth==16 but right now, it doesn't; reason: we don't
// store bitdepth==16 images, but only up to 8.
void PNGDecoder::applyTransparency(){
/* FIXME before copyPixels
  const FXColor CLEARALPHA=FXRGBA(255,255,255,0);
  if(alfacolor[0] && alfacolor[1] && alfacolor[2]  && (imagetype==RGB || imagetype==Gray)){
    FXuint npixels=width*height;
    FXColor color;
    for(FXuint i=0; i<npixels; ++i){
      if((color=image[i])==transparency){ image[i]=color&CLEARALPHA; }
      }
    }
*/
  }

/*******************************************************************************/

PERFORMANCE_RECORDER(PNGDecoder_decodeIndex1BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeIndex2BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeIndex4BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeIndex8BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeGray1BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeGray2BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeGray4BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeGray8BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeGray16BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeGrayAlfa8BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeGrayAlfa16BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeRGB8BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeRGB16BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeRGBA8BPP);
PERFORMANCE_RECORDER(PNGDecoder_decodeRGBA16BPP);


// Decode 1 bit/pixel indexed
static void decodeIndex1BPP(FXColor* dst,const PNGDecoder *dec,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeIndex1BPP);
  FXuchar w;
  while(8<=n){
    w=*src++;
    *dst=dec->colormap[w>>7].c;     dst+=s;
    *dst=dec->colormap[1&(w>>6)].c; dst+=s;
    *dst=dec->colormap[1&(w>>5)].c; dst+=s;
    *dst=dec->colormap[1&(w>>4)].c; dst+=s;
    *dst=dec->colormap[1&(w>>3)].c; dst+=s;
    *dst=dec->colormap[1&(w>>2)].c; dst+=s;
    *dst=dec->colormap[1&(w>>1)].c; dst+=s;
    *dst=dec->colormap[1&w].c;      dst+=s;
    n-=8;
    }
  if(0<n){ w=*src; *dst=dec->colormap[w>>7].c;     dst+=s;
  if(1<n){         *dst=dec->colormap[1&(w>>6)].c; dst+=s;
  if(2<n){         *dst=dec->colormap[1&(w>>5)].c; dst+=s;
  if(3<n){         *dst=dec->colormap[1&(w>>4)].c; dst+=s;
  if(4<n){         *dst=dec->colormap[1&(w>>3)].c; dst+=s;
  if(5<n){         *dst=dec->colormap[1&(w>>2)].c; dst+=s;
  if(6<n){         *dst=dec->colormap[1&(w>>1)].c; dst+=s; }}}}}}}
  }


// Decode 2 bit/pixel indexed
static void decodeIndex2BPP(FXColor* dst,const PNGDecoder *dec,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeIndex2BPP);
  FXuchar w;
  while(4<=n){
    w=*src++;
    *dst=dec->colormap[w>>6].c;     dst+=s;
    *dst=dec->colormap[3&(w>>4)].c; dst+=s;
    *dst=dec->colormap[3&(w>>2)].c; dst+=s;
    *dst=dec->colormap[3&w].c;      dst+=s;
    n-=4;
    }
  if(0<n){ w=*src; *dst=dec->colormap[w>>6].c;     dst+=s;
  if(1<n){         *dst=dec->colormap[3&(w>>4)].c; dst+=s;
  if(2<n){         *dst=dec->colormap[3&(w>>2)].c; dst+=s; }}}
  }


// Decode 4 bit/pixel indexed
static void decodeIndex4BPP(FXColor* dst,const PNGDecoder *dec,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeIndex4BPP);
  FXuchar w;
  while(2<=n){
    w=*src++;
    *dst=dec->colormap[w>>4].c; dst+=s;
    *dst=dec->colormap[w&15].c; dst+=s;
    n-=2;
    }
  if(0<n){ w=*src; *dst=dec->colormap[w>>4].c; }
  }


// Decode 8 bit/pixel indexed
static void decodeIndex8BPP(FXColor* dst,const PNGDecoder *dec,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeIndex8BPP);
  FXuchar w;
  while(n){
    w=*src++;
    *dst=dec->colormap[w].c;
    dst+=s;
    n--;
    }
  }


// Decode 1 bit/pixel gray
static void decodeGray1BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeGray1BPP);
  FXuchar w;
  while(8<=n){
    w=*src++;
    *dst=map1Bit[w>>7];     dst+=s;
    *dst=map1Bit[1&(w>>6)]; dst+=s;
    *dst=map1Bit[1&(w>>5)]; dst+=s;
    *dst=map1Bit[1&(w>>4)]; dst+=s;
    *dst=map1Bit[1&(w>>3)]; dst+=s;
    *dst=map1Bit[1&(w>>2)]; dst+=s;
    *dst=map1Bit[1&(w>>1)]; dst+=s;
    *dst=map1Bit[1&w];      dst+=s;
    n-=8;
    }
  if(0<n){ w=*src; *dst=map1Bit[w>>7];     dst+=s;
  if(1<n){         *dst=map1Bit[1&(w>>6)]; dst+=s;
  if(2<n){         *dst=map1Bit[1&(w>>5)]; dst+=s;
  if(3<n){         *dst=map1Bit[1&(w>>4)]; dst+=s;
  if(4<n){         *dst=map1Bit[1&(w>>3)]; dst+=s;
  if(5<n){         *dst=map1Bit[1&(w>>2)]; dst+=s;
  if(6<n){         *dst=map1Bit[1&(w>>1)]; dst+=s; }}}}}}}
  }


// Decode 2 bit/pixel gray
static void decodeGray2BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeGray2BPP);
  FXuchar w;
  while(4<=n){
    w=*src++;
    *dst=map2Bit[w>>6];     dst+=s;
    *dst=map2Bit[3&(w>>4)]; dst+=s;
    *dst=map2Bit[3&(w>>2)]; dst+=s;
    *dst=map2Bit[3&w];      dst+=s;
    n-=4;
    }
  if(0<n){ w=*src; *dst=map2Bit[w>>6];     dst+=s;
  if(1<n){         *dst=map2Bit[3&(w>>4)]; dst+=s;
  if(2<n){         *dst=map2Bit[3&(w>>2)]; dst+=s; }}}
  }


// Decode 4 bit/pixel gray
static void decodeGray4BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeGray4BPP);
  FXuchar w;
  while(2<=n){
    w=*src++;
    *dst=map4Bit[w>>4]; dst+=s;
    *dst=map4Bit[w&15]; dst+=s;
    n-=2;
    }
  if(0<n){ w=*src; *dst=map4Bit[w>>4]; }
  }


// Decode 8 bit/pixel gray
static void decodeGray8BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeGray8BPP);
  FXuchar g;
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__)
  const __m128i gr1=_mm_set_epi8(-1, 3, 3, 3, -1, 2, 2, 2, -1, 1, 1, 1,-1, 0, 0, 0);
  const __m128i gr2=_mm_set_epi8(-1, 7, 7, 7, -1, 6, 6, 6, -1, 5, 5, 5,-1, 4, 4, 4);
  const __m128i alfa=_mm_set_epi8(255,0,0,0, 255,0,0,0, 255,0,0,0, 255,0,0,0);
  __m128i xxxx,aaaa,bbbb;
  while(8<=n){
    xxxx=_mm_loadu_si64(src); src+=8;
    aaaa=_mm_or_si128(_mm_shuffle_epi8(xxxx,gr1),alfa);
    *dst=_mm_extract_epi32(aaaa,0); dst+=s;
    *dst=_mm_extract_epi32(aaaa,1); dst+=s;
    *dst=_mm_extract_epi32(aaaa,2); dst+=s;
    *dst=_mm_extract_epi32(aaaa,3); dst+=s;
    bbbb=_mm_or_si128(_mm_shuffle_epi8(xxxx,gr2),alfa);
    *dst=_mm_extract_epi32(bbbb,0); dst+=s;
    *dst=_mm_extract_epi32(bbbb,1); dst+=s;
    *dst=_mm_extract_epi32(bbbb,2); dst+=s;
    *dst=_mm_extract_epi32(bbbb,3); dst+=s;
    n-=8;
    }
#endif
  while(n){
    g=src[0];
    *dst=FXRGB(g,g,g);
    dst+=s;
    src+=1;
    n--;
    }
  }

// Decode 16 bit/pixel gray
static void decodeGray16BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeGray16BPP);
  FXushort g;
  while(n){
    g=((src[0]<<8)|src[1])/257;
    *dst=FXRGBA(g,g,g,255);
    dst+=s;
    src+=2;
    n--;
    }
  }


// Decode 8 bit/pixel gray-alpha
static void decodeGrayAlfa8BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeGrayAlfa8BPP);
  FXuchar g,a;
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__)
  const __m128i gr1=_mm_set_epi8( 7, 6, 6, 6,  5, 4, 4, 4,  3, 2, 2, 2, 1, 0, 0, 0);
  const __m128i gr2=_mm_set_epi8(15,14,14,14, 13,12,12,12, 11,10,10,10, 9, 8, 8, 8);
  __m128i xxxx,aaaa,bbbb;
  while(8<=n){
    xxxx=_mm_loadu_si128((const __m128i*)src); src+=16;
    aaaa=_mm_shuffle_epi8(xxxx,gr1);
    *dst=_mm_extract_epi32(aaaa,0); dst+=s;
    *dst=_mm_extract_epi32(aaaa,1); dst+=s;
    *dst=_mm_extract_epi32(aaaa,2); dst+=s;
    *dst=_mm_extract_epi32(aaaa,3); dst+=s;
    bbbb=_mm_shuffle_epi8(xxxx,gr2);
    *dst=_mm_extract_epi32(bbbb,0); dst+=s;
    *dst=_mm_extract_epi32(bbbb,1); dst+=s;
    *dst=_mm_extract_epi32(bbbb,2); dst+=s;
    *dst=_mm_extract_epi32(bbbb,3); dst+=s;
    n-=8;
    }
#endif
  while(n){
    g=src[0];
    a=src[1];
    *dst=FXRGBA(g,g,g,a);
    dst+=s;
    src+=2;
    n--;
    }
  }


// Decode 16 bit/pixel gray-alpha
static void decodeGrayAlfa16BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeGrayAlfa16BPP);
  FXushort g,a;
  while(n){
    g=((src[0]<<8)|src[1])/257;
    a=((src[2]<<8)|src[3])/257;
    *dst=FXRGBA(g,g,g,a);
    dst+=s;
    src+=4;
    n--;
    }
  }


// Decode 8 bit/pixel rgb
static void decodeRGB8BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeRGB8BPP);
  FXuchar r,g,b;
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__)
  const __m128i shuf=_mm_set_epi8(-1, 9,10,11, -1, 6, 7, 8, -1, 3, 4, 5, -1, 0, 1, 2);
  const __m128i alfa=_mm_set_epi8(255,0,0,0, 255,0,0,0, 255,0,0,0, 255,0,0,0);
  __m128i aaaa,bbbb,cccc,xxxx,yyyy,zzzz,wwww;
  while(16<=n){
    aaaa=_mm_loadu_si128((const __m128i*)src); src+=16;
    bbbb=_mm_loadu_si128((const __m128i*)src); src+=16;
    cccc=_mm_loadu_si128((const __m128i*)src); src+=16;
    xxxx=_mm_or_si128(_mm_shuffle_epi8(aaaa,shuf),alfa);
    yyyy=_mm_or_si128(_mm_shuffle_epi8(_mm_alignr_epi8(bbbb,aaaa,12),shuf),alfa);
    zzzz=_mm_or_si128(_mm_shuffle_epi8(_mm_alignr_epi8(cccc,bbbb,8),shuf),alfa);
    wwww=_mm_or_si128(_mm_shuffle_epi8(_mm_alignr_epi8(cccc,cccc,4),shuf),alfa);
    *dst=_mm_extract_epi32(xxxx,0); dst+=s;
    *dst=_mm_extract_epi32(xxxx,1); dst+=s;
    *dst=_mm_extract_epi32(xxxx,2); dst+=s;
    *dst=_mm_extract_epi32(xxxx,3); dst+=s;
    *dst=_mm_extract_epi32(yyyy,0); dst+=s;
    *dst=_mm_extract_epi32(yyyy,1); dst+=s;
    *dst=_mm_extract_epi32(yyyy,2); dst+=s;
    *dst=_mm_extract_epi32(yyyy,3); dst+=s;
    *dst=_mm_extract_epi32(zzzz,0); dst+=s;
    *dst=_mm_extract_epi32(zzzz,1); dst+=s;
    *dst=_mm_extract_epi32(zzzz,2); dst+=s;
    *dst=_mm_extract_epi32(zzzz,3); dst+=s;
    *dst=_mm_extract_epi32(wwww,0); dst+=s;
    *dst=_mm_extract_epi32(wwww,1); dst+=s;
    *dst=_mm_extract_epi32(wwww,2); dst+=s;
    *dst=_mm_extract_epi32(wwww,3); dst+=s;
    n-=16;
    }
#endif
  while(n){
    r=src[0];
    g=src[1];
    b=src[2];
    *dst=FXRGB(r,g,b);
    dst+=s;
    src+=3;
    n--;
    }
  }

// Decode 16 bit/pixel rgb
static void decodeRGB16BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeRGB16BPP);
  FXushort r,g,b;
  while(n){
    r=((src[0]<<8)|src[1])/257;
    g=((src[2]<<8)|src[3])/257;
    b=((src[4]<<8)|src[5])/257;
    *dst=FXRGB(r,g,b);
    dst+=s;
    src+=6;
    n--;
    }
  }


// Decode 8 bit/pixel rgba
static void decodeRGBA8BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeRGBA8BPP);
  FXuchar r,g,b,a;
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__)
  const __m128i bgra=_mm_set_epi8(15,12,13,14, 11,8,9,10, 7,4,5,6, 3,0,1,2);
  __m128i aaaa,bbbb;
  while(8<=n){
    aaaa=_mm_loadu_si128((const __m128i*)src); src+=16;
    aaaa=_mm_shuffle_epi8(aaaa,bgra);
    bbbb=_mm_loadu_si128((const __m128i*)src); src+=16;
    bbbb=_mm_shuffle_epi8(bbbb,bgra);
    *dst=_mm_extract_epi32(aaaa,0); dst+=s;
    *dst=_mm_extract_epi32(aaaa,1); dst+=s;
    *dst=_mm_extract_epi32(aaaa,2); dst+=s;
    *dst=_mm_extract_epi32(aaaa,3); dst+=s;
    *dst=_mm_extract_epi32(bbbb,0); dst+=s;
    *dst=_mm_extract_epi32(bbbb,1); dst+=s;
    *dst=_mm_extract_epi32(bbbb,2); dst+=s;
    *dst=_mm_extract_epi32(bbbb,3); dst+=s;
    n-=8;
    }
#endif
  while(n){
    r=src[0];
    g=src[1];
    b=src[2];
    a=src[3];
    *dst=FXRGBA(r,g,b,a);
    dst+=s;
    src+=4;
    n--;
    }
  }


// Decode 16 bit/pixel rgba
static void decodeRGBA16BPP(FXColor* dst,const PNGDecoder *,const FXuchar* src,FXuval n,FXuval s){
  PERFORMANCE_COUNTER(PNGDecoder_decodeRGBA16BPP);
  FXushort r,g,b,a;
  while(n){
    r=((src[0]<<8)|src[1])/257;
    g=((src[2]<<8)|src[3])/257;
    b=((src[4]<<8)|src[5])/257;
    a=((src[6]<<8)|src[7])/257;
    *dst=FXRGBA(r,g,b,a);
    dst+=s;
    src+=8;
    n--;
    }
  }

typedef void (*DecodeFunc)(FXColor*,const PNGDecoder *,const FXuchar*,FXuval,FXuval);

// Function table of mapping image_type and log(bits) to decode function
static const DecodeFunc decodeFunc[7][5]={
  {decodeGray1BPP,decodeGray2BPP,decodeGray4BPP,decodeGray8BPP,decodeGray16BPP},        // Gray
  {nullptr,nullptr,nullptr,nullptr,nullptr},                                            // N/A
  {nullptr,nullptr,nullptr,decodeRGB8BPP,decodeRGB16BPP},                               // RGB
  {decodeIndex1BPP,decodeIndex2BPP,decodeIndex4BPP,decodeIndex8BPP,nullptr},            // Index
  {nullptr,nullptr,nullptr,decodeGrayAlfa8BPP,decodeGrayAlfa16BPP},                     // Gray Alpha
  {nullptr,nullptr,nullptr,nullptr,nullptr},                                            // N/A
  {nullptr,nullptr,nullptr,decodeRGBA8BPP,decodeRGBA16BPP}                              // RGBA
  };

/*******************************************************************************/

// Alan Paeth's predictor
// CMOV (branch-free) version
static inline FXuchar predictor(FXshort a,FXshort b,FXshort c){
  FXshort A=b-c;
  FXshort B=a-c;
  FXshort C=A+B;
  A=Math::iabs(A);
  B=Math::iabs(B);
  C=Math::iabs(C);
  if(B<A){a=b;}         // CMOV
  if(B<A){A=B;}         // CMOV
  if(C<A){a=c;}         // CMOV
  return a;
  }


#if 0
// Decode one line
// . c b . . .
// . a x . . .
//static
static void decodeLine2(FXuchar filt,FXuchar* __restrict dst,const FXuchar* __restrict px,const FXuchar* __restrict pa,const FXuchar* __restrict pb,const FXuchar* __restrict pc,FXuval count,FXuval step){
  FXuval i=0;
  switch(filt){
  case FiltNone:
    do{ dst[i]=px[i]; }while(++i<count);
    return;
  case FiltSub:
    do{ dst[i]=px[i]; }while(++i<step);
    do{ dst[i]=px[i]+pa[i]; }while(++i<count);
    return;
  case FiltUp:
    if(pb){
      do{ dst[i]=px[i]+pb[i]; }while(++i<count);
      }
    else{
      do{ dst[i]=px[i]; }while(++i<count);
      }
    return;
  case FiltAvg:
    if(pb){
      do{ dst[i]=px[i]+(pb[i]>>1); }while(++i<step);
      do{ dst[i]=px[i]+((pa[i]+pb[i])>>1); }while(++i<count);
      }
    else{
      do{ dst[i]=px[i]; }while(++i<step);
      do{ dst[i]=px[i]+(pa[i]>>1); }while(++i<count);
      }
    return;
  case FiltPaeth:
    if(pb){
      do{ dst[i]=px[i]+pb[i]; }while(++i<step);
      do{ dst[i]=px[i]+predictor(pa[i],pb[i],pc[i]); }while(++i<count);
      }
    else{
      do{ dst[i]=px[i]; }while(++i<step);
      do{ dst[i]=px[i]+pa[i]; }while(++i<count);
      }
    return;
    }
  __unreachable();
  }
#endif

PERFORMANCE_RECORDER(PNGDecoder_decodeLine);

// Decode one line
static void decodeLine(FXuchar filt,FXuchar* __restrict cur,const FXuchar* __restrict prv,FXuval count,FXuval step){
  PERFORMANCE_COUNTER(PNGDecoder_decodeLine);
  switch(filt){
  case FiltNone:
    return;
  case FiltSub:
    for(FXuval i=step; i<count; ++i){
      cur[i]+=cur[i-step];
      }
    return;
  case FiltUp:
    if(prv){
      for(FXuval i=0; i<count; ++i){
        cur[i]+=prv[i];
        }
      }
    return;
  case FiltAvg:
    if(prv){
      for(FXuval i=0; i<step; ++i){
        cur[i]+=prv[i]/2;
        }
      for(FXuval i=step; i<count; ++i){
        cur[i]+=(cur[i-step]+prv[i])/2;
        }
      }
    else{
      for(FXuval i=step; i<count; ++i){
        cur[i]+=(cur[i-step])/2;
        }
      }
    return;
  case FiltPaeth:
    if(prv){
      for(FXuval i=0; i<step; ++i){
        cur[i]+=prv[i];
        }
      for(FXuval i=step; i<count; ++i){
        cur[i]+=predictor(cur[i-step],prv[i],prv[i-step]);
        }
      }
    else{
      for(FXuval i=step; i<count; ++i){
        cur[i]+=cur[i-step];
        }
      }
    return;
  default:
    __unreachable();
    }
  }

/*******************************************************************************/

PERFORMANCE_RECORDER(PNGDecoder_decode);


// Decode image
FXbool PNGDecoder::decode(){
  PERFORMANCE_COUNTER(PNGDecoder_decode);
  DecodeFunc df=decodeFunc[imagetype][logBitdepth[bitdepth]];
  FXuchar* cur=buffer;
  FXuchar* prv=nullptr;
  FXColor* dst;
  FXuchar  filt;

  // Interlaced mode
  if(interlace==Adam7){
    for(FXuint pass=0; pass<7; prv=nullptr,++pass){
      for(FXuint row=0; row<intheight[pass]; ++row){
        filt=*cur++;
        if(__unlikely(FiltPaeth<filt)) return false;
        decodeLine(filt,cur,prv,intbytes[pass],stride);
        dst=image+xoffset[pass]+((yoffset[pass]+(ystep[pass]*row))*width);
        df(dst,this,cur,intwidth[pass],xstep[pass]);
        prv=cur;
        cur+=intbytes[pass];
        }
      }
    }

  // Normal mode
  else{
    for(FXuint row=0; row<height; ++row){
      filt=*cur++;
      if(__unlikely(FiltPaeth<filt)) return false;
      decodeLine(filt,cur,prv,numbytes,stride);
      dst=image+(row*width);
      df(dst,this,cur,width,1);
      prv=cur;
      cur+=numbytes;
      }
    }
  return true;
  }


PERFORMANCE_RECORDER(PNGDecoder_data);
PERFORMANCE_RECORDER(PNGDecoder_inflate);


//  buffer         next_out
//   |               |
//   |               |           next_in
//   |               |             |
//   |               |             |
//   +---------------+-------------+-------------+
//   | uncompressed  | inflating   | compressed  |
//   +---------------+-------------+-------------+
//   |               |             |             |
//   |               |             |             |
//   |               |<-avail_out->|<-avail_in-->|
//   |                                           |
//   |                                           |
//   |<-------------- buffersize --------------->|

// Load data
// The zlib inflate() is set up decompress the entire image to the START
// of the buffer, while the compressed data is loaded to the END of the buffer.
// The gap [stream.next_in-stream.next_out] is available to receive the newly
// decompressed bytes.
// Thus, the stream.next_out pointer CHASES the stream.next_in pointer, and
// may sometimes catch up with it.
// If this happens the inflate() simply is invoked again after updating
// stream.avail_out, since stream.next_in is advanced toward the end of the
// buffer as data is uncompressed.
// All bytes in the IDAT chunk must be CRCed, even if the expected IDAT length
// exceeds the expected amount for the full image (totbytes). However, inflate()
// stops further processing of data when the full image has been decompressed.
FXbool PNGDecoder::data(FXStream& store,FXuint length){
  PERFORMANCE_COUNTER(PNGDecoder_data);
  FXuint crc=CRC32::CRC(~0,IDAT);
  FXuint chunkcrc=0;
  FXint  zstatus=Z_OK;

  FXTRACE((TOPIC_DETAIL,"fxloadPNG: IDAT length: %u\n",length));

  // Loop to consume this chunk
  while(0<length){

    // The uncompressed image is totbytes, so grabbing up to that
    // size has a good probability of getting the entire image.
    // If there's more data, we'll need to go around the loop a 2nd
    // time, possibly without decompressing any further data [just
    // so we can compute its CRC].
    stream.avail_in=FXMIN(length,totbytes);

    // We now have room for [transfer] new data
    stream.next_in=buffer+buffersize-stream.avail_in;

    // Load compressed block
    store.load(stream.next_in,stream.avail_in);

    // Calculate CRC
    crc=CRC32::CRC(crc,stream.next_in,stream.avail_in);

    // Update remaining unconsumed part of the chunk
    length-=stream.avail_in;

    // Potentially loop since we may hit stream.next_in from behind
    while(0<stream.avail_in && (stream.next_out-buffer)<totbytes){
      PERFORMANCE_COUNTER(PNGDecoder_inflate);
      stream.avail_out=stream.next_in-stream.next_out;
      zstatus=inflate(&stream,Z_NO_FLUSH);
      if(zstatus<=Z_ERRNO){
        FXTRACE((TOPIC_DETAIL,"fxloadPNG: inflate returned: %d\n",zstatus));
        return false;
        }
      FXTRACE((TOPIC_DETAIL,"fxloadPNG: inflated = %u/%u\n",(FXuint)(stream.next_out-buffer),totbytes));
      }
    }

  // Checksum
  store >> chunkcrc;
  if(chunkcrc!=~crc){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: IDAT crc mismatch\n"));
    return false;
    }
  return (store.status()==FXStreamOK);
  }


// Load end
FXbool PNGDecoder::end(FXStream& store,FXuint length){
  FXuint crc=CRC32::CRC(~0,IEND);
  FXuint chunkcrc=0;

  FXTRACE((TOPIC_DETAIL,"fxloadPNG: IEND length: %u\n",length));

  // Are we short?
  if((stream.next_out-buffer)<totbytes){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: expected: %u bytes, got only %u\n",totbytes,(FXuint)(stream.next_out-buffer)));
    return false;
    }

  // Decode
  if(!decode()){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: IEND bad filter selection\n"));
    return false;
    }

  // Apply alpha color
  applyTransparency();

  // Checksum
  store >> chunkcrc;
  if(chunkcrc!=~crc){
    FXTRACE((TOPIC_DETAIL,"fxloadPNG: IEND crc mismatch\n"));
    return false;
    }

  // OK!
  return true;
  }


PERFORMANCE_RECORDER(PNGDecoder_load);


// Decode start
FXbool PNGDecoder::load(FXStream& store,FXColor*& output_image,FXint& output_width,FXint& output_height){
  PERFORMANCE_COUNTER(PNGDecoder_load);
  FXbool result=false;
  FXuint chunklength;
  FXuint chunkid;

  // Load header
  if(header(store)){

    // Initialize zlib
    if(inflateInit(&stream)==Z_OK){

      // Finally, image to be returned
      if(allocElms(image,width*height)){

        // Size of buffer 1.25 x totbytes + 2 x numbytes + 2
        buffersize=((totbytes*5+3)>>2)+((numbytes+1)<<1);

        FXTRACE((TOPIC_DETAIL,"fxloadPNG: buffersize = %u\n",buffersize));

        // Allocate storage
        if(allocElms(buffer,buffersize)){

          // Uncompressed data goes here
          stream.next_out=buffer;

          // Parse chunks
          while(!store.eof()){

            // Get chunk
            store >> chunklength;
            store >> chunkid;

            // Palette
            if(chunkid==PLTE){
              if(!palette(store,chunklength)){
                FXTRACE((TOPIC_DETAIL,"fxloadPNG: failed to load PLTE\n"));
                break;
                }
              continue;
              }

            // Background
            if(chunkid==bKGD){
              if(!background(store,chunklength)){
                FXTRACE((TOPIC_DETAIL,"fxloadPNG: failed to load bKGD\n"));
                break;
                }
              continue;
              }

            // Transparancy
            if(chunkid==tRNS){
              if(!transparency(store,chunklength)){
                FXTRACE((TOPIC_DETAIL,"fxloadPNG: failed to load tRNS\n"));
                break;
                }
              continue;
              }

            // Data
            if(chunkid==IDAT){
              if(!data(store,chunklength)){
                FXTRACE((TOPIC_DETAIL,"fxloadPNG: failed to load IDAT\n"));
                break;
                }
              continue;
              }

            // End
            if(chunkid==IEND){
              if(!end(store,chunklength)){
                FXTRACE((TOPIC_DETAIL,"fxloadPNG: failed to load IEND\n"));
                break;
                }

              // Copy to out
              output_image=image;
              output_width=width;
              output_height=height;
              image=nullptr;
              result=true;
              break;
              }

            // Other chunk
            // Skip over the data and the crc
            store.position(chunklength+4,FXFromCurrent);
            }
          freeElms(buffer);
          }
        freeElms(image);
        }

      // Close down zlib
      inflateEnd(&stream);
      }
    }
  FXTRACE((TOPIC_DETAIL,"fxloadPNG: %s\n\n",result?"ok":"failed"));
  return result;
  }


PERFORMANCE_RECORDER(fxloadPNG);


// Load a PNG image
FXbool fxloadPNG(FXStream& store,FXColor*& data,FXint& width,FXint& height){
  PERFORMANCE_COUNTER(fxloadPNG);
  FXbool result=false;
  data=nullptr;
  width=0;
  height=0;
  if(store.direction()==FXStreamLoad){
    FXuchar sig[8]={0,0,0,0,0,0,0,0};
    FXbool swap=store.swapBytes();
    store.setBigEndian(true);
    store.load(sig,8);
    if(equalElms(sig,signature,8)){
      PNGDecoder decoder;
      result=decoder.load(store,data,width,height);
      }
    store.swapBytes(swap);
    }
  return result;
  }


// Check if stream contains a PNG
FXbool fxcheckPNG(FXStream& store){
  if(store.direction()==FXStreamLoad){
    FXuchar sig[ARRAYNUMBER(signature)];
    store.load(sig,ARRAYNUMBER(signature));
    store.position(-ARRAYNUMBER(signature),FXFromCurrent);
    return equalElms(sig,signature,ARRAYNUMBER(signature));
    }
  return false;
  }

/*******************************************************************************/

// PNG Encoder
class PNGEncoder {
public:
  const FXColor*image;
  FXuchar      *buffer;                 // Zlib output buffer
  FXuint        buffersize;             // Zlib output buffer size
  z_stream      stream;                 // Zlib stream
  FXuint        width;                  // Width
  FXuint        height;                 // Height
  FXuchar       imagetype;              // Image type (Gray, Indexed, ...)
  FXuchar       bitdepth;               // Bit depth per channel
  FXuchar       compression;            // Compression mode
  FXuchar       filter;                 // Filter
  FXuchar       interlace;              // Interlace (NoInterlace, Adam7)
  FXuint        stride;                 // Pixel stride
  FXuint        totbytes;               // Number of total bytes
  FXuint        numbytes;               // Number of bytes/line
  FXuint        ncolormap;              // Number of colors in colormap
  RGBAPixel     colormap[256];          // Colormap
  RGBAPixel     colortable[512];        // Color to index hash table
  FXushort      indextable[512];        // Indexes assigned for each color
  FXushort      alfacolor[3];           // Alpha color spec
public:
  FXuint analyze() const;
  FXbool mapcolors();
  FXuint index(FXColor color) const;
  FXbool unused();
  FXbool header(FXStream& store);
  FXbool palette(FXStream& store) const;
  FXbool transparency(FXStream& store) const;
  FXbool encode(FXuint flags);
  FXbool data(FXStream& store);
  FXbool end(FXStream& store);
public:

  // Initialize encoder
  PNGEncoder():image(nullptr),buffer(nullptr),buffersize(0),width(0),height(0),imagetype(0),bitdepth(8),compression(Deflate),filter(FiltNone),interlace(NoInterlace),stride(0),totbytes(0),numbytes(0),ncolormap(0){
    alfacolor[0]=alfacolor[1]=alfacolor[2]=0;
    clearElms(&stream,1);
    clearElms(colormap,ARRAYNUMBER(colormap));
    clearElms(colortable,ARRAYNUMBER(colortable));
    clearElms(indextable,ARRAYNUMBER(indextable));
    }

  // Save image
  FXbool save(FXStream& store,const FXColor* img,FXint w,FXint h,FXuint flags);
  };


PERFORMANCE_RECORDER(PNGEncoder_analyze);


// Analyze image for special cases:
//
//  PNG_IMAGE_GRAY    all pixels have r==g==b.
//  PNG_IMAGE_OPAQUE  all pixels are opaque.
//
FXuint PNGEncoder::analyze() const {
  PERFORMANCE_COUNTER(PNGEncoder_analyze);
  const FXColor* end=image+width*height;
  const FXColor* ptr=image;
  FXuint flags=PNG_IMAGE_GRAY|PNG_IMAGE_OPAQUE;
  FXColor color;
  while(flags && ptr!=end){
    color=*ptr++;
    if(FXBLUEVAL(color)!=FXREDVAL(color)){flags&=~PNG_IMAGE_GRAY;}
    if(FXBLUEVAL(color)!=FXGREENVAL(color)){flags&=~PNG_IMAGE_GRAY;}
    if(FXALPHAVAL(color)<255){flags&=~PNG_IMAGE_OPAQUE;}
    }
  return flags;
  }


// Find unused color, an (r,g,b) triplet that does not occur in
// the image.  This may be used to encode full-transparent alpha
// in case we have either fully transparent or fully opaque pixels,
// saving us the trouble of writing a boring alpha channel.
FXbool PNGEncoder::unused(){
  const FXColor* end=image+width*height;
  const FXColor* ptr=image;
  FXbool result=false;
  FXuchar used[3][256];
  FXColor color;
  clearElms(used,ARRAYNUMBER(used));
  while(ptr!=end){
    color=*ptr++;
    used[0][FXBLUEVAL(color)]=1;
    used[1][FXGREENVAL(color)]=1;
    used[2][FXREDVAL(color)]=1;
    }
  for(FXuint i=0; i<256; ++i){
    if(used[0][i]==0){ alfacolor[2]=i; result=true; }
    if(used[1][i]==0){ alfacolor[1]=i; result=true; }
    if(used[2][i]==0){ alfacolor[0]=i; result=true; }
    }
  return result;
  }


// Simple but quite fast "hash" function
static inline FXuint HashColor(FXColor clr){
  return clr*0x9E3779B1;
  }


// Find index given color in table; note that we now know the
// color to be in the map, so only loop exit is at the location
// of the color [no un-initialized slots will be visited].
FXuint PNGEncoder::index(FXColor color) const {
  FXuint p,b,x;
  p=b=HashColor(color);
  while(__unlikely(colortable[x=p&511].c!=color)){
    p=(p<<2)+p+b+1;
    b>>=5;
    }
  return indextable[x];
  }


// Collate by (a) alpha, and then (b) blue value.
// Rationale: allow us to discover if image is gray ramp. If it is,
// and grey values are uniform, then we can omit the palette and just
// write gray imagetype.
static inline FXbool operator>(const RGBAPixel& p,const RGBAPixel& q){
  return (p.a>q.a) || ((p.a==q.a) && (p.b>q.b));
  }


// Arrange colors by increasing alpha, so opaque pixels
// would appear at the END of the map, allowing the list
// of alpha-values to be shorter than the whole palette.
static void sortmap(RGBAPixel colormap[],FXint n){
  RGBAPixel color;
  FXint s=1,i,j;
  while(s<=n/9){ s=3*s+1; }
  while(0<s){
    for(i=s+1; i<=n; ++i){
      color=colormap[i-1];
      for(j=i; s<j && (colormap[j-s-1]>color); j-=s){
        colormap[j-1]=colormap[j-s-1];
        }
      colormap[j-1]=color;
      }
    s/=3;
    }
  }


PERFORMANCE_RECORDER(PNGEncoder_mapcolors);


// Collect colors of the image into a colormap; bail if there's more
// than 256 individual colors; otherwise, sort the colors and assign
// each color an index into the sorted list.
FXbool PNGEncoder::mapcolors(){
  PERFORMANCE_COUNTER(PNGEncoder_mapcolors);
  const FXColor ALPHA=FXRGBA(0,0,0,255);
  FXuint  npixels=width*height;
  FXuint  ncolors=0;
  FXuint  i,p,b,x;
  FXColor color;

  // Hash all colors from image
  for(i=0; i<npixels; ++i){

    // Get pixel
    color=image[i];

    // Find color's position in table
    p=b=HashColor(color);

    // Locate color in map; adding it if not yet seen
    while(__likely(indextable[x=p&511])){
      if(__likely(colortable[x].c==color)) goto nxt;
      p=(p<<2)+p+b+1;
      b>>=5;
      }

    // Not fitting
    if(__unlikely(ncolors>=256)) return false;

    // Color to index mappings
    // Count number of non-opaque colors; if only
    // opaque colors are present, we may not need
    // to save a transparancy list at all!
    FXASSERT(x<512);
    colortable[x].c=color;
    indextable[x]=1;

    // Add to map
    colormap[ncolors].c=color;
    ncolors++;

    // Next pixel
nxt:continue;
    }

  // Sort map by alpha
  sortmap(colormap,ncolors);

  // Now assign colors to index; note we now
  // know each color MUST be in the map!
  for(i=0; i<ncolors; ++i){
    color=colormap[i].c;
    p=b=HashColor(color);
    while(colortable[x=p&511].c!=color){
      p=(p<<2)+p+b+1;
      b>>=5;
      }
    indextable[x]=i;
    }
  ncolormap=ncolors;
  return true;
  }


// Save header
FXbool PNGEncoder::header(FXStream& store){
  FXuint crc=CRC32::CRC(~0,IHDR);
  FXuint length=13;
  store << length;
  store << IHDR;
  store << width;
  store << height;
  store << bitdepth;
  store << imagetype;
  store << compression;
  store << filter;
  store << interlace;
  crc=CRC32::CRC(crc,width);
  crc=CRC32::CRC(crc,height);
  crc=CRC32::CRC(crc,bitdepth);
  crc=CRC32::CRC(crc,imagetype);
  crc=CRC32::CRC(crc,compression);
  crc=CRC32::CRC(crc,filter);
  crc=CRC32::CRC(crc,interlace);
  crc^=~0;
  store << crc;
  return (store.status()==FXStreamOK);
  }


PERFORMANCE_RECORDER(PNGEncoder_palette);

// Save colormap
FXbool PNGEncoder::palette(FXStream& store) const {
  PERFORMANCE_COUNTER(PNGEncoder_palette);
  FXuint crc=CRC32::CRC(~0,PLTE);
  FXuint length=ncolormap*3;
  store << length;
  store << PLTE;
  for(FXuint i=0; i<ncolormap; ++i){
    store << colormap[i].r;
    store << colormap[i].g;
    store << colormap[i].b;
    crc=CRC32::CRC(crc,colormap[i].r);
    crc=CRC32::CRC(crc,colormap[i].g);
    crc=CRC32::CRC(crc,colormap[i].b);
    }
  crc^=~0;
  store << crc;
  return (store.status()==FXStreamOK);
  }


PERFORMANCE_RECORDER(PNGEncoder_transparency);


// Save transparancy
FXbool PNGEncoder::transparency(FXStream& store) const {
  PERFORMANCE_COUNTER(PNGEncoder_transparency);
  FXuint crc=CRC32::CRC(~0,tRNS);
  FXuint length=ncolormap;
  store << length;
  store << tRNS;
  for(FXuint i=0; i<ncolormap; ++i){
    store << colormap[i].a;
    crc=CRC32::CRC(crc,colormap[i].a);
    }
  crc^=~0;
  store << crc;
  return (store.status()==FXStreamOK);
  }

/*******************************************************************************/


PERFORMANCE_RECORDER(PNGEncoder_encodeIndex1BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeIndex2BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeIndex4BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeIndex8BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeGray1BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeGray2BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeGray4BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeGray8BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeGrayAlfa8BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeRGB8BPP);
PERFORMANCE_RECORDER(PNGEncoder_encodeRGBA8BPP);


// Encode 1 bit/pixel indexed
static void encodeIndex1BPP(FXuchar* dst,const PNGEncoder* enc,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeIndex1BPP);
  FXuchar s;
  while(8<=n){
    s= enc->index(src[0])<<7;
    s|=enc->index(src[1])<<6;
    s|=enc->index(src[2])<<5;
    s|=enc->index(src[3])<<4;
    s|=enc->index(src[4])<<3;
    s|=enc->index(src[5])<<2;
    s|=enc->index(src[6])<<1;
    s|=enc->index(src[7]);
    *dst++=s;
    src+=8;
    n-=8;
    }
  if(1<=n){ s= enc->index(src[0])<<7;
  if(2<=n){ s|=enc->index(src[1])<<6;
  if(3<=n){ s|=enc->index(src[2])<<5;
  if(4<=n){ s|=enc->index(src[3])<<4;
  if(5<=n){ s|=enc->index(src[4])<<3;
  if(6<=n){ s|=enc->index(src[5])<<2;
  if(7<=n){ s|=enc->index(src[6])<<1; }}}}}} *dst=s; }
  }


// Encode 2 bit/pixel indexed
static void encodeIndex2BPP(FXuchar* dst,const PNGEncoder* enc,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeIndex2BPP);
  FXuchar s;
  while(4<=n){
    s= enc->index(src[0])<<6;
    s|=enc->index(src[1])<<4;
    s|=enc->index(src[2])<<2;
    s|=enc->index(src[3]);
    *dst++=s;
    src+=4;
    n-=4;
    }
  if(1<=n){ s= enc->index(src[0])<<6;
  if(2<=n){ s|=enc->index(src[1])<<4;
  if(3<=n){ s|=enc->index(src[2])<<2; }} *dst=s; }
  }


// Encode 4 bit/pixel indexed
static void encodeIndex4BPP(FXuchar* dst,const PNGEncoder* enc,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeIndex4BPP);
  while(2<=n){
    *dst++=(enc->index(src[0])<<4) | (enc->index(src[1]));
    src+=2;
    n-=2;
    }
  if(1<=n){ *dst++=enc->index(src[0]); }
  }


// Encode 8 bit/pixel indexed
static void encodeIndex8BPP(FXuchar* dst,const PNGEncoder* enc,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeIndex8BPP);
  while(n){
    *dst++=enc->index(*src);
    src++;
    n--;
    }
  }


// Encode 1 bit/pixel gray
static void encodeGray1BPP(FXuchar* dst,const PNGEncoder*,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeGray1BPP);
  FXuchar w;
  while(8<=n){
    w= ((FXBLUEVAL(src[0]))&0x80);
    w|=((FXBLUEVAL(src[1])>>1)&0x40);
    w|=((FXBLUEVAL(src[2])>>2)&0x20);
    w|=((FXBLUEVAL(src[3])>>3)&0x10);
    w|=((FXBLUEVAL(src[4])>>4)&0x08);
    w|=((FXBLUEVAL(src[5])>>5)&0x04);
    w|=((FXBLUEVAL(src[6])>>6)&0x02);
    w|=((FXBLUEVAL(src[7])>>7)&0x01);
    *dst++=w;
    src+=8;
    n-=8;
    }
  if(1<=n){ w= ((FXBLUEVAL(src[0]))&0x80);
  if(2<=n){ w|=((FXBLUEVAL(src[1])>>1)&0x40);
  if(3<=n){ w|=((FXBLUEVAL(src[2])>>2)&0x20);
  if(4<=n){ w|=((FXBLUEVAL(src[3])>>3)&0x10);
  if(5<=n){ w|=((FXBLUEVAL(src[4])>>4)&0x08);
  if(6<=n){ w|=((FXBLUEVAL(src[5])>>5)&0x04);
  if(7<=n){ w|=((FXBLUEVAL(src[6])>>6)&0x02); }}}}}} *dst=w; }
  }


// Encode 2 bit/pixel gray
static void encodeGray2BPP(FXuchar* dst,const PNGEncoder*,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeGray2BPP);
  FXuchar w;
  while(4<=n){
    w= ((FXBLUEVAL(src[0]))&0xC0);
    w|=((FXBLUEVAL(src[1])>>2)&0x30);
    w|=((FXBLUEVAL(src[2])>>4)&0x0C);
    w|=((FXBLUEVAL(src[3])>>6)&0x03);
    *dst++=w;
    src+=4;
    n-=4;
    }
  if(1<=n){ w= ((FXBLUEVAL(src[0]))&0xC0);
  if(2<=n){ w|=((FXBLUEVAL(src[1])>>2)&0x30);
  if(3<=n){ w|=((FXBLUEVAL(src[2])>>4)&0x0C); }} *dst=w; }
  }


// Encode 4 bit/pixel gray
static void encodeGray4BPP(FXuchar* dst,const PNGEncoder*,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGDecoder_decodeGray4BPP);
  FXuchar w;
  while(2<=n){
    w= FXBLUEVAL(src[0])&0xF0;
    w|=FXBLUEVAL(src[1])>>4;
    *dst++=w;
    src+=2;
    n-=2;
    }
  if(1<=n){ *dst++=(FXBLUEVAL(src[0])&0xF0); }
  }


// Encode 8 bit/pixel gray
static void encodeGray8BPP(FXuchar* dst,const PNGEncoder*,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeGray8BPP);
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__)
  const __m128i gray=_mm_set_epi8(-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,12,8,4,0);
  __m128i aaaa,bbbb;
  while(8<=n){
    aaaa=_mm_loadu_si128((const __m128i*)src); src+=4;
    bbbb=_mm_loadu_si128((const __m128i*)src); src+=4;
    aaaa=_mm_shuffle_epi8(aaaa,gray);
    bbbb=_mm_shuffle_epi8(bbbb,gray);
    _mm_storeu_si32(dst,aaaa); dst+=4;
    _mm_storeu_si32(dst,bbbb); dst+=4;
    n-=8;
    }
#endif
  while(n){
    *dst++=FXBLUEVAL(src[0]);
    src++;
    n--;
    }
  }



// Encode 8 bit/channel gray-alpha
static void encodeGrayAlfa8BPP(FXuchar* dst,const PNGEncoder*,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeGrayAlfa8BPP);
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__)
  const __m128i gray=_mm_set_epi8(-1,-1,-1,-1,-1,-1,-1,-1,15,12,11,8,7,4,3,0);
  __m128i aaaa,bbbb;
  while(8<=n){
    aaaa=_mm_loadu_si128((const __m128i*)src); src+=4;
    bbbb=_mm_loadu_si128((const __m128i*)src); src+=4;
    aaaa=_mm_shuffle_epi8(aaaa,gray);
    bbbb=_mm_shuffle_epi8(bbbb,gray);
    _mm_storeu_si64(dst,aaaa); dst+=8;
    _mm_storeu_si64(dst,bbbb); dst+=8;
    n-=8;
    }
#endif
  while(n){
    *dst++=FXBLUEVAL(*src);
    *dst++=FXALPHAVAL(*src);
    src++;
    n--;
    }
  }


// Encode 8 bit/channel rgb
static void encodeRGB8BPP(FXuchar* dst,const PNGEncoder*,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeRGB8BPP);
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__)
  const __m128i shuf=_mm_set_epi8(-1,-1,-1,-1,12,13,14,8,9,10,4,5,6,0,1,2);
  __m128i aaaa,bbbb,cccc,dddd;
  while(16<=n){
    aaaa=_mm_loadu_si128((const __m128i*)src); src+=4;
    bbbb=_mm_loadu_si128((const __m128i*)src); src+=4;
    cccc=_mm_loadu_si128((const __m128i*)src); src+=4;
    dddd=_mm_loadu_si128((const __m128i*)src); src+=4;
    aaaa=_mm_shuffle_epi8(aaaa,shuf);
    bbbb=_mm_shuffle_epi8(bbbb,shuf);
    cccc=_mm_shuffle_epi8(cccc,shuf);
    dddd=_mm_shuffle_epi8(dddd,shuf);
    aaaa=_mm_or_si128(aaaa,_mm_slli_si128(bbbb,12));
    bbbb=_mm_or_si128(_mm_srli_si128(bbbb,4),_mm_slli_si128(cccc,8));
    cccc=_mm_or_si128(_mm_srli_si128(cccc,8),_mm_slli_si128(dddd,4));
    _mm_storeu_si128((__m128i*)dst,aaaa); dst+=16;
    _mm_storeu_si128((__m128i*)dst,bbbb); dst+=16;
    _mm_storeu_si128((__m128i*)dst,cccc); dst+=16;
    n-=16;
    }
#endif
  while(n){
    *dst++=FXREDVAL(*src);
    *dst++=FXGREENVAL(*src);
    *dst++=FXBLUEVAL(*src);
    src++;
    n--;
    }
  }


// Encode 8 bit/channel rgba
static void encodeRGBA8BPP(FXuchar* dst,const PNGEncoder*,const FXColor* src,FXuval n){
  PERFORMANCE_COUNTER(PNGEncoder_encodeRGBA8BPP);
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__)
  const __m128i rgba=_mm_set_epi8(15,12,13,14, 11,8,9,10, 7,4,5,6, 3,0,1,2);
  __m128i aaaa,bbbb;
  while(8<=n){
    aaaa=_mm_loadu_si128((const __m128i*)src); src+=4;
    bbbb=_mm_loadu_si128((const __m128i*)src); src+=4;
    aaaa=_mm_shuffle_epi8(aaaa,rgba);
    bbbb=_mm_shuffle_epi8(bbbb,rgba);
    _mm_storeu_si128((__m128i*)dst,aaaa); dst+=16;
    _mm_storeu_si128((__m128i*)dst,bbbb); dst+=16;
    n-=8;
    }
#endif
  while(n){
    *dst++=FXREDVAL(*src);
    *dst++=FXGREENVAL(*src);
    *dst++=FXBLUEVAL(*src);
    *dst++=FXALPHAVAL(*src);
    src++;
    n--;
    }
  }


typedef void (*EncodeFunc)(FXuchar*,const PNGEncoder* enc,const FXColor*,FXuval);

// Function table of mapping image_type and log(bits) to decode function
static const EncodeFunc encodeFunc[7][5]={
  {encodeGray1BPP,encodeGray2BPP,encodeGray4BPP,encodeGray8BPP,nullptr},        // Gray
  {nullptr,nullptr,nullptr,nullptr,nullptr},                                    // N/A
  {nullptr,nullptr,nullptr,encodeRGB8BPP,nullptr},                              // RGB
  {encodeIndex1BPP,encodeIndex2BPP,encodeIndex4BPP,encodeIndex8BPP,nullptr},    // Index
  {nullptr,nullptr,nullptr,encodeGrayAlfa8BPP,nullptr},                         // Gray Alpha
  {nullptr,nullptr,nullptr,nullptr,nullptr},                                    // N/A
  {nullptr,nullptr,nullptr,encodeRGBA8BPP,nullptr}                              // RGBA
  };


/*******************************************************************************/

PERFORMANCE_RECORDER(PNGEncoder_encodeLine);


// Encoding one line
static void encodeLine(FXuchar filt,FXuchar*  __restrict dst,const FXuchar*  __restrict cur,const FXuchar* __restrict prv,FXuval count,FXuval step){
  PERFORMANCE_COUNTER(PNGEncoder_encodeLine);
  FXuval i=0;
  switch(filt){
  case FiltNone:
    do{ dst[i]=cur[i]; }while(++i<count);
    return;
  case FiltSub:
    do{ dst[i]=cur[i]; }while(++i<step);
    do{ dst[i]=cur[i]-cur[i-step]; }while(++i<count);
    return;
  case FiltUp:
    do{ dst[i]=cur[i]-prv[i]; }while(++i<count);
    return;
  case FiltAvg:
    do{ dst[i]=cur[i]-(prv[i]>>1); }while(++i<step);
    do{ dst[i]=cur[i]-((cur[i-step]+prv[i])>>1); }while(++i<count);
    return;
  case FiltPaeth:
    do{ dst[i]=cur[i]-prv[i]; }while(++i<step);
    do{ dst[i]=cur[i]-predictor(cur[i-step],prv[i],prv[i-step]); }while(++i<count);
    return;
  default:
    __unreachable();
    }
  }


// Calculate score, an indication of how well the line may be compressed
// Nicely vectorizes with pabsb/pmovzxbw/pmovzxwd on x86-64-v3...
static inline FXint calculateScore(const FXuchar* pix,FXuval n){
  FXint result=0;
  while(n){ result+=Math::iabs((FXint)(FXschar)*pix++); --n; }
  return result;
  }


// Determine best filter, given
static inline FXuchar findBestFilter(FXuchar* __restrict dst,const FXuchar* __restrict cur,const FXuchar* __restrict prv,FXuval count,FXuval step){
  FXint   bestsum=2147483647,sum;
  FXuchar bestflt=FiltNone;
  for(FXuchar flt=FiltNone; flt<=FiltPaeth; ++flt){
    encodeLine(flt,dst,cur,prv,count,step);
    sum=calculateScore(dst,count);
    if(sum<bestsum){ bestflt=flt; bestsum=sum; }
    }
  return bestflt;
  }

/*******************************************************************************/


PERFORMANCE_RECORDER(PNGEncoder_encode);


// Encode image to space at end of buffer, using two extra
// lines during the encoding, if needed.
// Only apply filter for bit-depths of at least 8; otherwise
// just copy to destination in buffer.
FXbool PNGEncoder::encode(FXuint flags){
  PERFORMANCE_COUNTER(PNGEncoder_encode);
  EncodeFunc ef=encodeFunc[imagetype][logBitdepth[bitdepth]];
  FXuchar*       dst=buffer+buffersize-totbytes;
  FXuchar*       prv=buffer;
  FXuchar*       cur=buffer+numbytes;
  const FXColor* src=image;
  FXuchar        flt=(flags&PNG_FILTER_MASK);

  // No filter, copy to destination directly
  if(flt==PNG_FILTER_NONE){
    for(FXuint row=0; row<height; ++row){
      ef(dst+1,this,src,width);
      dst[0]=FiltNone;
      dst+=numbytes+1;
      src+=width;
      }
    }

  // Find and use best filter
  else if(flt==PNG_FILTER_BEST){
    for(FXuint row=0; row<height; ++row){
      ef(cur,this,src,width);
      flt=findBestFilter(dst+1,cur,prv,numbytes,stride);
      encodeLine(flt,dst+1,cur,prv,numbytes,stride);
      dst[0]=flt;
      dst+=numbytes+1;
      src+=width;
      swap(prv,cur);
      }
    }

  // Encode with given filter
  else{
    for(FXuint row=0; row<height; ++row){
      ef(cur,this,src,width);
      encodeLine(flt,dst+1,cur,prv,numbytes,stride);
      dst[0]=flt;
      dst+=numbytes+1;
      src+=width;
      swap(prv,cur);
      }
    }
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: encoded: %u/%u\n",(FXuint)(dst-buffer-buffersize+totbytes),totbytes));
  return true;
  }


PERFORMANCE_RECORDER(PNGEncoder_data);
PERFORMANCE_RECORDER(PNGEncoder_deflate);


//  buffer         next_out
//   |               |
//   |               |           next_in
//   |               |             |
//   |               |             |
//   +---------------+-------------+-------------+
//   | compressed    | deflating   | compressed  |
//   +---------------+-------------+-------------+
//   |               |             |             |
//   |               |             |             |
//   |               |<-avail_out->|<-totbytes-->|
//   |                                           |
//   |                                           |
//   |<-------------- buffersize --------------->|

// Save image data
// New version writes one row at a time, and requires no
// backup to earlier point in the stream to overwrite chunksize.
FXbool PNGEncoder::data(FXStream& store){
  PERFORMANCE_COUNTER(PNGEncoder_data);
  FXuint crc=CRC32::CRC(~0,IDAT);
  FXint  zstatus=Z_OK;
  FXuint length;

  // Set up for compression
  stream.next_in=buffer+buffersize-totbytes;
  stream.avail_in=totbytes;
  stream.next_out=buffer;

  // Potentially loop since we may hit stream.next_in from behind
  while(0<stream.avail_in || zstatus!=Z_STREAM_END){
    PERFORMANCE_COUNTER(PNGEncoder_deflate);
    stream.avail_out=stream.next_in-stream.next_out;
    zstatus=deflate(&stream,Z_FINISH);          // 92% of the runtime of fsavePNG
    if(zstatus<=Z_ERRNO){
      FXTRACE((TOPIC_DETAIL,"fxsavePNG: deflate returned: %d\n",zstatus));
      return false;
      }
    FXTRACE((TOPIC_DETAIL,"fxsavePNG: deflated = %u/%u\n",(FXuint)(stream.next_in-buffer-buffersize+totbytes),totbytes));
    }

  // Compute compressed length
  length=(FXuint)(stream.next_out-buffer);

  FXTRACE((TOPIC_DETAIL,"fxsavePNG: IDAT len: %u\n",length));

  // Save chunk length and chunk id
  store << length;
  store << IDAT;

  // Save compressed block
  store.save(buffer,length);

  // Calculate CRC
  crc=CRC32::CRC(crc,buffer,length);

  // Save CRC
  crc^=~0;
  store << crc;
  return (store.status()==FXStreamOK);
  }


// Save IEND
FXbool PNGEncoder::end(FXStream& store){
  FXuint crc=CRC32::CRC(~0,IEND);
  FXuint length=0;
  store << length;
  store << IEND;
  crc^=~0;
  store << crc;
  return (store.status()==FXStreamOK);
  }


PERFORMANCE_RECORDER(PNGEncoder_save);


// Save image with encoder
FXbool PNGEncoder::save(FXStream& store,const FXColor* img,FXint w,FXint h,FXuint flags){
  PERFORMANCE_COUNTER(PNGEncoder_save);
  FXint  level=Z_DEFAULT_COMPRESSION;
  FXbool result=false;
  FXuint mode;
  FXuint ch;

  // Init image data
  image=img;
  width=w;
  height=h;
  bitdepth=8;
  imagetype=RGBA;
  compression=Deflate;
  filter=0;
  interlace=NoInterlace;

  // Analyze image if requested
  mode=flags;
  if(flags&PNG_IMAGE_ANALYZE){
    mode=analyze();
    }

  // Pick RGBA, RGB, GrayAlpha, or Gray
  if(mode&PNG_IMAGE_GRAY){
    imagetype=GrayAlpha;
    if(mode&PNG_IMAGE_OPAQUE) imagetype=Gray;
    }
  else{
    imagetype=RGBA;
    if(mode&PNG_IMAGE_OPAQUE) imagetype=RGB;
    }

  // Try indexed color mode
  if(flags&PNG_INDEX_COLOR){
    if(mapcolors()){
      imagetype=Indexed;
      if(ncolormap<=16) bitdepth=4;
      if(ncolormap<=4)  bitdepth=2;
      if(ncolormap<=2)  bitdepth=1;
      }
    }

  // Compute some handy values
  ch=channels[imagetype];
  numbytes=(width*ch*bitdepth+7)>>3;
  totbytes=numbytes*height+height;
  stride=(ch*bitdepth+7)>>3;

  FXTRACE((TOPIC_DETAIL,"fxsavePNG: width     = %u\n",width));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: height    = %u\n",height));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: bitdepth  = %u\n",bitdepth));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: imagetype = %s\n",imagetype==Gray?"Gray":imagetype==RGB?"RGB":imagetype==Indexed?"Indexed":imagetype==GrayAlpha?"GrayAlpha":imagetype==RGBA?"RGBA":"Invalid"));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: compress  = %u\n",compression));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: filter    = %u\n",filter));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: interlace = %s\n",interlace?"Adam7":"None"));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: channels  = %u\n",ch));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: totbytes  = %u\n",totbytes));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: numbytes  = %u\n",numbytes));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: stride    = %u\n",stride));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: ncolormap = %u\n",ncolormap));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: mode      = %04b\n",mode));
  FXTRACE((TOPIC_DETAIL,"fxsavePNG: flags     = %04b\n",flags));

  // Save header
  if(header(store)){

    // Save colormap and optional alpha map
    if(imagetype==Indexed){
      if(!palette(store)) goto x;
      if(!(mode&PNG_IMAGE_OPAQUE)){
        if(!transparency(store)) goto x;
        }
      }

    // Compression options
    if(flags&PNG_COMPRESS_FAST) level=Z_BEST_SPEED;
    if(flags&PNG_COMPRESS_BEST) level=Z_BEST_COMPRESSION;

    // Initialize zlib
    if(deflateInit(&stream,level)==Z_OK){

      // Enough space for deflate() totbytes, plus extra lines
      buffersize=deflateBound(&stream,totbytes+(numbytes<<2));

      FXTRACE((TOPIC_DETAIL,"fxsavePNG: buffersize = %u\n",buffersize));

      // Allocate storage
      if(callocElms(buffer,buffersize)){

        // Set filter to none if less than 2 bytes/pixel
        if(bitdepth<8){ flags&=~PNG_FILTER_MASK; }

        // Encode image
        encode(flags);

        // Save row
        if(data(store)){

          // Save terminator
          result=end(store);
          }

        // Free memory
        freeElms(buffer);
        }

      // Close down zlib
      deflateEnd(&stream);
      }
    }

  // Successfully saved
x:FXTRACE((TOPIC_DETAIL,"fxsavePNG: %s\n",result?"ok":"failed"));
  return result;
  }


PERFORMANCE_RECORDER(fxsavePNG);


// Save a PNG image
FXbool fxsavePNG(FXStream& store,const FXColor* data,FXint width,FXint height,FXuint flags){
  PERFORMANCE_COUNTER(fxsavePNG);
  FXbool result=false;
  if(store.direction()==FXStreamSave){
    if(data && 0<width && 0<height){
      FXbool swap=store.swapBytes();
      PNGEncoder encoder;
      store.setBigEndian(true);
      store.save(signature,8);
      result=encoder.save(store,data,width,height,flags);
      store.swapBytes(swap);
      }
    }
  return result;
  }

#else  //////////////////////////////////////////////////////////////////////////


// Check if stream contains a PNG
FXbool fxcheckPNG(FXStream&){
  return false;
  }


// Stub routine
FXbool fxloadPNG(FXStream&,FXColor*& data,FXint& width,FXint& height){
  static const FXColor color[2]={FXRGB(0,0,0),FXRGB(255,255,255)};
  static const FXuchar png_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x80, 0xfd, 0xff, 0xff, 0xbf,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0xc5, 0x23, 0xc4, 0xa1,
   0x45, 0x24, 0x24, 0xa2, 0x45, 0x64, 0x24, 0xa0, 0x45, 0xa4, 0x24, 0xa0,
   0x45, 0x24, 0x25, 0xa0, 0xc5, 0x23, 0x26, 0xa3, 0x45, 0x20, 0x24, 0xa2,
   0x45, 0x20, 0x24, 0xa2, 0x45, 0x20, 0xc4, 0xa1, 0x05, 0x00, 0x00, 0xa0,
   0x05, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00, 0xa0, 0xfd, 0xff, 0xff, 0xbf,
   0x01, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff};
  allocElms(data,32*32);
  for(FXint p=0; p<32*32; p++){
    data[p]=color[(png_bits[p>>3]>>(p&7))&1];
    }
  width=32;
  height=32;
  return true;
  }


// Stub routine
FXbool fxsavePNG(FXStream&,const FXColor*,FXint,FXint,FXuint){
  return false;
  }


#endif //////////////////////////////////////////////////////////////////////////

}


