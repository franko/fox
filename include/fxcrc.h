/********************************************************************************
*                                                                               *
*                           C R C 3 2   S u p p o r t                           *
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
#ifndef FXCRC_H
#define FXCRC_H

namespace FX {

namespace CRC32 {

/// Standard CRC table
extern FXAPI const FXuint crctab[256];

/**
* Calculate CRC32 using polynomial P(x) = 0x1EDB88320 over GF(2):
*
*  1) Start with: crc=~0; (set to 0xFFFFFFFF).
*
*  2) Divide bitstream by P, collection remainder:
*
*       crc=CRC32(crc,byte),
*       crc=CRC32(crc,short),
*       crc=CRC32(crc,int),                    (in any order)
*       crc=CRC32(crc,long),
*       crc=CRC32(crc,buffer,length),
*
*  3) Complement with: crc^=~0; (xor with 0xFFFFFFFF).
*
* Faster versions are possible; we'll have it soon.
* And no, you can't use x86 CPU's built-in crc32 instruction;
* it uses different polynomial P(x)=0x11EDC6F41 (the Castagnoli
* polynomial). But x86 CPUs have "carry-less" multiply instruction,
* which will make this code here a fallback option only useful
* on non-x86 CPUs.
*/

/// Calculate CRC32 of unsigned char
static inline FXuint CRC(FXuint crc,FXuchar byte){
  return crctab[(crc^byte)&0xff] ^ (crc>>8);
  }


/// Calculate CRC32 of unsigned short
static inline FXuint CRC(FXuint crc,FXushort x){
  union{ FXushort s; FXuchar b[2]; } z={x};
#if FOX_BIGENDIAN == 1
  crc=CRC(crc,z.b[0]);
  crc=CRC(crc,z.b[1]);
#else
  crc=CRC(crc,z.b[1]);
  crc=CRC(crc,z.b[0]);
#endif
  return crc;
  }


/// Calculate CRC32 of unsigned int
static inline FXuint CRC(FXuint crc,FXuint x){
  union{ FXuint i; FXuchar b[4]; } z={x};
#if FOX_BIGENDIAN == 1
  crc=CRC(crc,z.b[0]);
  crc=CRC(crc,z.b[1]);
  crc=CRC(crc,z.b[2]);
  crc=CRC(crc,z.b[3]);
#else
  crc=CRC(crc,z.b[3]);
  crc=CRC(crc,z.b[2]);
  crc=CRC(crc,z.b[1]);
  crc=CRC(crc,z.b[0]);
#endif
  return crc;
  }


/// Calculate CRC32 of unsigned long
static inline FXuint CRC(FXuint crc,FXulong x){
  union{ FXulong l; FXuchar b[8]; } z={x};
#if FOX_BIGENDIAN == 1
  crc=CRC(crc,z.b[0]);
  crc=CRC(crc,z.b[1]);
  crc=CRC(crc,z.b[2]);
  crc=CRC(crc,z.b[3]);
  crc=CRC(crc,z.b[4]);
  crc=CRC(crc,z.b[5]);
  crc=CRC(crc,z.b[6]);
  crc=CRC(crc,z.b[7]);
#else
  crc=CRC(crc,z.b[7]);
  crc=CRC(crc,z.b[6]);
  crc=CRC(crc,z.b[5]);
  crc=CRC(crc,z.b[4]);
  crc=CRC(crc,z.b[3]);
  crc=CRC(crc,z.b[2]);
  crc=CRC(crc,z.b[1]);
  crc=CRC(crc,z.b[0]);
#endif
  return crc;
  }


/// Calculate CRC32 of a array of unsigned chars
static inline FXuint CRC(FXuint crc,const FXuchar *buf,FXival len){
  const FXuchar* end=buf+len;
  while(buf!=end){
    crc=CRC(crc,*buf++);
    }
  return crc;
  }

}

}

#endif
