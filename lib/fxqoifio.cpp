/********************************************************************************
*                                                                               *
*                         Q O I F    I n p u t / O u t p u t                    *
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
#include "fxmath.h"
#include "FXElement.h"
#include "FXArray.h"
#include "FXMetaClass.h"
#include "FXHash.h"
#include "FXElement.h"
#include "FXStream.h"

/*
  Notes:

  - A QOIF file has a 14 byte header, followed by any number of data "chunks" and an
    8-byte end marker.

    struct qoi_header_t {
            char     magic[4];   // magic bytes "qoif"
            uint32_t width;      // image width in pixels (BE)
            uint32_t height;     // image height in pixels (BE)
            uint8_t  channels;   // 3 = RGB, 4 = RGBA
            uint8_t  colorspace; // 0 = sRGB with linear alpha, 1 = all channels linear
    };

  - Images are encoded row by row, left to right, top to bottom. The decoder and
    encoder start with {r: 0, g: 0, b: 0, a: 255} as the previous pixel value. An
    image is complete when all pixels specified by width * height have been covered.

  - Pixels are encoded as
     - a run of the previous pixel
     - an index into an array of previously seen pixels
     - a difference to the previous pixel value in r,g,b
     - full r,g,b or r,g,b,a values

  - The color channels are assumed to not be premultiplied with the alpha channel
    ("un-premultiplied alpha").

  - A running array[64] (zero-initialized) of previously seen pixel values is
    maintained by the encoder and decoder. Each pixel that is seen by the encoder
    and decoder is put into this array at the position formed by a hash function of
    the color value. In the encoder, if the pixel value at the index matches the
    current pixel, this index position is written to the stream as QOI_OP_INDEX.
    The hash function for the index is:

            index_position = (r * 3 + g * 5 + b * 7 + a * 11) % 64

  - Each chunk starts with a 2- or 8-bit tag, followed by a number of data bits. The
    bit length of chunks is divisible by 8 - i.e. all chunks are byte aligned. All
    values encoded in these data bits have the most significant bit on the left.

    The 8-bit tags have precedence over the 2-bit tags. A decoder must check for the
    presence of an 8-bit tag first.

    The byte stream's end is marked with 7 0x00 bytes followed a single 0x01 byte.


    The possible chunks are:


    .- QOI_OP_INDEX ----------.
    |         Byte[0]         |
    |  7  6  5  4  3  2  1  0 |
    |-------+-----------------|
    |  0  0 |     index       |
    `-------------------------`
    2-bit tag b00
    6-bit index into the color index array: 0..63

  - A valid encoder must not issue 2 or more consecutive QOI_OP_INDEX chunks to the
    same index. QOI_OP_RUN should be used instead.


    .- QOI_OP_DIFF -----------.
    |         Byte[0]         |
    |  7  6  5  4  3  2  1  0 |
    |-------+-----+-----+-----|
    |  0  1 |  dr |  dg |  db |
    `-------------------------`
    2-bit tag b01
    2-bit   red channel difference from the previous pixel between -2..1
    2-bit green channel difference from the previous pixel between -2..1
    2-bit  blue channel difference from the previous pixel between -2..1

  - The difference to the current channel values are using a wraparound operation,
    so "1 - 2" will result in 255, while "255 + 1" will result in 0.

    Values are stored as unsigned integers with a bias of 2. E.g. -2 is stored as
    0 (b00). 1 is stored as 3 (b11).

    The alpha value remains unchanged from the previous pixel.


    .- QOI_OP_LUMA -------------------------------------.
    |         Byte[0]         |         Byte[1]         |
    |  7  6  5  4  3  2  1  0 |  7  6  5  4  3  2  1  0 |
    |-------+-----------------+-------------+-----------|
    |  1  0 |  green diff     |   dr - dg   |  db - dg  |
    `---------------------------------------------------`
    2-bit tag b10
    6-bit green channel difference from the previous pixel -32..31
    4-bit   red channel difference minus green channel difference -8..7
    4-bit  blue channel difference minus green channel difference -8..7

  - The green channel is used to indicate the general direction of change and is
    encoded in 6 bits. The red and blue channels (dr and db) base their diffs off
    of the green channel difference and are encoded in 4 bits. I.e.:
            dr_dg = (cur_px.r - prev_px.r) - (cur_px.g - prev_px.g)
            db_dg = (cur_px.b - prev_px.b) - (cur_px.g - prev_px.g)

    The difference to the current channel values are using a wraparound operation,
    so "10 - 13" will result in 253, while "250 + 7" will result in 1.

    Values are stored as unsigned integers with a bias of 32 for the green channel
    and a bias of 8 for the red and blue channel.

    The alpha value remains unchanged from the previous pixel.


    .- QOI_OP_RUN ------------.
    |         Byte[0]         |
    |  7  6  5  4  3  2  1  0 |
    |-------+-----------------|
    |  1  1 |       run       |
    `-------------------------`
    2-bit tag b11
    6-bit run-length repeating the previous pixel: 1..62

  - The run-length is stored with a bias of -1. Note that the run-lengths 63 and 64
    (b111110 and b111111) are illegal as they are occupied by the QOI_OP_RGB and
    QOI_OP_RGBA tags.


    .- QOI_OP_RGB ------------------------------------------.
    |         Byte[0]         | Byte[1] | Byte[2] | Byte[3] |
    |  7  6  5  4  3  2  1  0 | 7 .. 0  | 7 .. 0  | 7 .. 0  |
    |-------------------------+---------+---------+---------|
    |  1  1  1  1  1  1  1  0 |   red   |  green  |  blue   |
    `-------------------------------------------------------`
    8-bit tag b11111110
    8-bit   red channel value
    8-bit green channel value
    8-bit  blue channel value

    The alpha value remains unchanged from the previous pixel.


    .- QOI_OP_RGBA ---------------------------------------------------.
    |         Byte[0]         | Byte[1] | Byte[2] | Byte[3] | Byte[4] |
    |  7  6  5  4  3  2  1  0 | 7 .. 0  | 7 .. 0  | 7 .. 0  | 7 .. 0  |
    |-------------------------+---------+---------+---------+---------|
    |  1  1  1  1  1  1  1  1 |   red   |  green  |  blue   |  alpha  |
    `-----------------------------------------------------------------`
    8-bit tag b11111111
    8-bit   red channel value
    8-bit green channel value
    8-bit  blue channel value
    8-bit alpha channel value
*/

#define TOPIC_DETAIL 1015

using namespace FX;

/*******************************************************************************/

namespace FX {


#ifndef FXLOADQOIF
extern FXAPI FXbool fxcheckQOIF(FXStream& store);
extern FXAPI FXbool fxloadQOIF(FXStream& store,FXColor*& data,FXint& width,FXint& height);
extern FXAPI FXbool fxsaveQOIF(FXStream& store,const FXColor* data,FXint width,FXint height);
#endif


// Chunk types
const FXuchar QOI_OP_INDEX = 0x00;      // 00xxxxxx
const FXuchar QOI_OP_DIFF  = 0x40;      // 01xxxxxx
const FXuchar QOI_OP_LUMA  = 0x80;      // 10xxxxxx
const FXuchar QOI_OP_RUN   = 0xC0;      // 11xxxxxx
const FXuchar QOI_OP_RGB   = 0xFE;      // 11111110
const FXuchar QOI_OP_RGBA  = 0xFF;      // 11111111
const FXuchar QOI_OP_MASK  = 0xC0;      // 11000000


// A pixel
union RGBAType {
  struct { FXuchar b,g,r,a; } rgba;
  FXuint v;
  };


// PNG file signature
static const FXuchar signature[4]={'q','o','i','f'};


// End of image padding
static const FXuchar padding[8]={0,0,0,0,0,0,0,1};


// Hash pixel
static inline FXuint hashRBGA(const RGBAType& p){
  return p.rgba.b*7+p.rgba.g*5+p.rgba.r*3+p.rgba.a*11;
  }


// Check if stream contains a QOI
FXbool fxcheckQOIF(FXStream& store){
  if(store.direction()==FXStreamLoad){
    FXuchar head[4]={0,0,0,0};
    store.load(head,4);
    store.position(-4,FXFromCurrent);
    return (head[0]==signature[0]) && (head[1]==signature[1]) && (head[2]==signature[2]) && (head[3]==signature[3]);
    }
  return false;
  }


// Load a QOI image
FXbool fxloadQOIF(FXStream& store,FXColor*& data,FXint& width,FXint& height){

  // Null out
  data=nullptr;
  width=0;
  height=0;

  // Is loading
  if(store.direction()==FXStreamLoad){
    FXbool   swap=store.swapBytes();
    FXuchar  head[4]={0,0,0,0};
    FXuchar  tail[8]={0,0,0,0,0,0,0,0};
    RGBAType index[64];
    RGBAType pixel;
    FXColor* imgdata;
    FXuint   imgwidth;
    FXuint   imgheight;
    FXuint   alpha;
    FXuint   size;
    FXuchar  channels;     // 3 = RGB, 4 = RGBA
    FXuchar  colorspace;   // 0 = sRGB with linear alpha, 1 = all channels linear
    FXint    vg;
    FXuint   run=0;
    FXuint   p=0;
    FXuint   h;
    FXuchar  byte;

    // Big endian
    store.setBigEndian(true);

    // Load signature
    store.load(head,4);

    // Check signature
    if((head[0]==signature[0]) && (head[1]==signature[1]) && (head[2]==signature[2]) && (head[3]==signature[3])){

      // Get image info
      store >> imgwidth;
      store >> imgheight;
      store >> channels;
      store >> colorspace;

      // Total size
      size=imgwidth*imgheight;

      FXTRACE((TOPIC_DETAIL,"fxloadQOIF: %d x %d\n",width,height));

      // Proper numbers
      if((channels==3 || channels==4) && (colorspace==0 || colorspace==1)){

        // Size is sane
        if(0<imgwidth && 0<imgheight && imgheight<(2147483647/imgwidth)){

          // Allocate data
          if(allocElms(imgdata,size)){

            // Zero out array
            clearElms(index,ARRAYNUMBER(index));

            pixel.rgba.b=0;
            pixel.rgba.g=0;
            pixel.rgba.r=0;
            pixel.rgba.a=255;

            // Set alpha to 255 if no apha channel
            alpha=(channels==3) ? FXRGBA(0,0,0,255) : FXRGBA(0,0,0,0);

            // Decode the chunks
            while(p<size){

              // Grab next chunk
              if(run==0){
                store >> byte;
                if(byte==QOI_OP_RGB){
                  store >> pixel.rgba.r;
                  store >> pixel.rgba.g;
                  store >> pixel.rgba.b;
                  }
                else if(byte==QOI_OP_RGBA){
                  store >> pixel.rgba.r;
                  store >> pixel.rgba.g;
                  store >> pixel.rgba.b;
                  store >> pixel.rgba.a;
                  }
                else if((byte&QOI_OP_MASK)==QOI_OP_INDEX){
                  pixel=index[byte];
                  }
                else if((byte&QOI_OP_MASK)==QOI_OP_DIFF){
                  pixel.rgba.r+=((byte>>4)&3)-2;
                  pixel.rgba.g+=((byte>>2)&3)-2;
                  pixel.rgba.b+=(byte&3)-2;
                  }
                else if((byte&QOI_OP_MASK)==QOI_OP_LUMA){
                  vg=-32+(byte&0x3F);
                  store >> byte;
                  pixel.rgba.r+=vg-8+(byte>>4);
                  pixel.rgba.g+=vg;
                  pixel.rgba.b+=vg-8+(byte&15);
                  }
                else if((byte&QOI_OP_MASK)==QOI_OP_RUN){
                  run=(byte&0x3F);
                  }

                // Calculate hash
                h=hashRBGA(pixel)&0x3F;

                // Remember last pixel in hash
                index[h]=pixel;
                }

              // Still in run
              else{
                --run;
                }

              // Assign pixel
              imgdata[p++]=(pixel.v|alpha);
              }

            // Parse over tail
            store.load(tail,8);

            // Loaded tail
            if(tail[7]==1){
              FXTRACE((TOPIC_DETAIL,"fxloadQOIF: ok\n"));
              store.swapBytes(swap);
              data=imgdata;
              width=imgwidth;
              height=imgheight;
              return (store.status()==FXStreamOK);
              }

            // Epic fail
            freeElms(imgdata);
            }
          }
        }
      }
    store.swapBytes(swap);
    }
  return false;
  }


/*******************************************************************************/

// Save a QOI image
FXbool fxsaveQOIF(FXStream& store,const FXColor* data,FXint width,FXint height){

  // Is saving and something to save
  if(store.direction()==FXStreamSave){
    if(data && 0<width && 0<height && height<(2147483647/width)){
      FXbool   swap=store.swapBytes();
      RGBAType index[64];
      RGBAType pixel;
      RGBAType prior;
      FXuint   size=width*height;
      FXuchar  channels=4;
      FXuchar  colorspace=1;
      FXuchar  byte;
      FXschar  vr;
      FXschar  vg;
      FXschar  vb;
      FXschar  vg_r;
      FXschar  vg_b;
      FXuint   run=0;
      FXuint   p=0;
      FXuint   h;

      FXTRACE((TOPIC_DETAIL,"fxsaveQOIF: %d x %d\n",width,height));

      // Big endian
      store.setBigEndian(true);

      // Start with PNG signature
      store.save(signature,4);

      // Save image info
      store << width;
      store << height;
      store << channels;
      store << colorspace;

      // Zero out array
      clearElms(index,ARRAYNUMBER(index));

      // Init pixels
      pixel.rgba.b=0;
      pixel.rgba.g=0;
      pixel.rgba.r=0;
      pixel.rgba.a=255;
      prior=pixel;

      // Encode to chunks
      while(p<size){
        pixel.v=data[p++];
        if(pixel.v==prior.v){
          ++run;
          if(run==62 || p==size){
            byte=QOI_OP_RUN | (run-1);
            store << byte;
            run=0;
            }
          }
        else{
          if(run){
            byte=QOI_OP_RUN | (run-1);
            store << byte;
            run=0;
            }

          // Calculate hash
          h=hashRBGA(pixel)&0x3F;

          // Was in table?
          if(index[h].v==pixel.v){
            byte=QOI_OP_INDEX | h;
            store << byte;
            }

          // Nope, store and encode another way
          else{
            index[h]=pixel;
            if(pixel.rgba.a==prior.rgba.a){
              vr=pixel.rgba.r-prior.rgba.r;
              vg=pixel.rgba.g-prior.rgba.g;
              vb=pixel.rgba.b-prior.rgba.b;
              vg_r=vr-vg;
              vg_b=vb-vg;
              if(vr>-3 && vr<2 && vg>-3 && vg<2 && vb>-3 && vb<2){
                byte=QOI_OP_DIFF | (vr+2)<<4 | (vg+2)<<2 | (vb+2);
                store << byte;
                }
              else if(vg_r>-9 && vg_r<8 && vg>-33 && vg<32 && vg_b>-9 && vg_b<8){
                byte=QOI_OP_LUMA | (vg+32);
                store << byte;
                byte=(vg_r+8)<<4 | (vg_b+8);
                store << byte;
                }
              else{
                store << QOI_OP_RGB;
                store << pixel.rgba.r;
                store << pixel.rgba.g;
                store << pixel.rgba.b;
                }
              }
            else{
              store << QOI_OP_RGB;
              store << pixel.rgba.r;
              store << pixel.rgba.g;
              store << pixel.rgba.b;
              store << pixel.rgba.a;
              }
            }
          }
        prior=pixel;
        }

      // Tail end padding
      store.save(padding,8);

      FXTRACE((TOPIC_DETAIL,"fxsaveQOIF: ok\n"));

      // Reset byte order
      store.swapBytes(swap);
      return (store.status()==FXStreamOK);
      }
    }
  return false;
  }

}
